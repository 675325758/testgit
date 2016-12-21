#include "cl_priv.h"
#include "area_priv.h"
#include "scene_priv.h"
#include "linkage_client.h"


// 最大一千多万，暂时不考虑翻转情况
cl_handle_t handle_create(int type)
{
	cl_handle_t h;
	
	cl_lock(&cl_priv->mutex);
		
	cl_priv->handles[type]++;
	if (cl_priv->handles[type] >= HANDLE_MAX)
		cl_priv->handles[type] = 1;

	h = ((type & 0xFF) << 24) | cl_priv->handles[type];
	
	cl_unlock(&cl_priv->mutex);

	return h;
}

/*
	各种查找
*/

user_t *user_lookup_by_sn(u_int64_t sn)
{
	user_t *user, *dev;
	
	stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
		if (user->sn == sn)
			return user;

		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				if (dev->sn == sn)
					return dev;
			}
		}
	}

	return NULL;
}

slave_t *slave_lookup_by_ident(u_int64_t sn)
{
    user_t *user;
    slave_t* slave;
    
    stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
        slave = slave_lookup_by_sn(user, sn);
        if (slave) {
            return slave;
        }
    }
    return NULL;
}

slave_t *slave_lookup_by_sn(user_t *user, u_int64_t sn)
{
	slave_t *slave, *dev_slave;
	user_t *dev;
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->sn == sn)
			return slave;

		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				stlc_list_for_each_entry(slave_t, dev_slave, &dev->slave, link) {
					if (dev_slave->sn == sn)
						return dev_slave;
				}
			}
		}
	}

	log_err(false, "not found slave by sn=%012llu\n", sn);

	return NULL;
}

slave_t *slave_lookup_by_handle(user_t *user, cl_handle_t handle)
{
	slave_t *slave, *dev_slave;
	user_t *dev;
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->handle == handle)
			return slave;

		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				stlc_list_for_each_entry(slave_t, dev_slave, &dev->slave, link) {
					if (dev_slave->handle == handle)
						return dev_slave;
				}
			}
		}
	}

	log_err(false, "not found slave by handle=0x%08x\n", handle);

	return NULL;
}

video_t *video_lookup_by_handle(user_t *user, cl_handle_t handle)
{
	slave_t *slave, *dev_slave;
	user_t *dev;
	
	stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
		if (slave->video != NULL && slave->video->handle == handle)
			return slave->video;

		if (user->is_phone_user) {
			stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
				stlc_list_for_each_entry(slave_t, dev_slave, &dev->slave, link) {
					if (dev_slave->video != NULL && dev_slave->video->handle == handle)
						return dev_slave->video;
				}
			}
		}
	}

	log_err(false, "not found slave by handle=0x%08x\n", handle);

	return NULL;
}

void *lookup_by_handle(u_int8_t lookup_type, cl_handle_t handle)
{
	u_int8_t type;
	user_t *user, *dev;
	slave_t *slave;
    equipment_t* eq;
    area_priv_t* area_priv;
    scene_priv_t* scene_priv;
	tmp_obj_t *to;
	la_user_t *puser;
	la_home_t *phome;
	
	type = (handle >> 24) & 0xFF;
	/* 先放松对摄像头的检查，因为现在003用的是从设备的类型 */
	if (lookup_type != HDLT_VIDEO &&  type != lookup_type) {
		log_err(false, "lookup_by_handle failed: not same type, lookup_type=%u, handle type=%u, handle=0x%08x\n",
			lookup_type, type, handle);
		return NULL;
	}
	
	switch (lookup_type) {
	case HDLT_USER:
		stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
			if (user->handle == handle)
				return (void *)user;

			if (user->is_phone_user) {
				stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
					if (dev->handle == handle)
						return (void *)dev;
				}
			}
		}
		break;

	case HDLT_SLAVE:
		stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
			stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
				if (slave->handle == handle)
					return (void *)slave;
			}

			if (user->is_phone_user) {
				stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
					stlc_list_for_each_entry(slave_t, slave, &dev->slave, link) {
						if (slave->handle == handle)
							return (void *)slave;
					}
				}
			}
		}
		break;

	case HDLT_VIDEO:
		stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
			stlc_list_for_each_entry(slave_t, slave, &user->slave, link) {
				if (slave->video != NULL && slave->video->handle == handle)
					return (void *)slave->video;
			}

			if (user->is_phone_user) {
				stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
					stlc_list_for_each_entry(slave_t, slave, &dev->slave, link) {
						if (slave->video != NULL && slave->video->handle == handle)
							return (void *)slave->video;
					}
				}
			}
		}
		break;
		
    case HDLT_EQUIPMENT:
        stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
            if (!stlc_list_empty(&user->slave)) {
                slave = stlc_list_first_entry(&user->slave, slave_t, link);
                if (slave->equipment!=NULL && !stlc_list_empty(&slave->equipment->eq_list)) {
                    stlc_list_for_each_entry(equipment_t, eq, &slave->equipment->eq_list, link){
                        if (eq->handle == handle) {
                            return (void*)eq;
                        }
                    }
                    
                }
            }

			if (user->is_phone_user) {
				stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
		            if (!stlc_list_empty(&dev->slave)) {
		                slave = stlc_list_first_entry(&dev->slave, slave_t, link);
		                if (slave->equipment!=NULL && !stlc_list_empty(&slave->equipment->eq_list)) {
		                    stlc_list_for_each_entry(equipment_t, eq, &slave->equipment->eq_list, link){
		                        if (eq->handle == handle) {
		                            return (void*)eq;
		                        }
		                    }
		                    
		                }
		            }
				}
			}
        }
        break;
		
    case HDLT_AREA:
        stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
            if (user->ac && !stlc_list_empty(&user->ac->area_list)) {
                stlc_list_for_each_entry(area_priv_t,area_priv,&user->ac->area_list,link){
                    if (area_priv->area_handle == handle) {
                        return (void*)area_priv;
                    }
                }
				
            }
			if (user->is_phone_user) {
				stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
					if (dev->ac == NULL)
						continue;
	                stlc_list_for_each_entry(area_priv_t, area_priv, &dev->ac->area_list, link){
	                    if (area_priv->area_handle == handle) {
	                        return (void*)area_priv;
	                    }
	                }
				}
			}
        }
        break;
		
    case HDLT_SCENE:
        stlc_list_for_each_entry(user_t, user, &cl_priv->user, link) {
            if (user->sc && !stlc_list_empty(&user->sc->scene_list)) {
                stlc_list_for_each_entry(scene_priv_t,scene_priv,&user->sc->scene_list,link){
                    if (scene_priv->scene_handle == handle) {
                        return (void*)scene_priv;
                    }
                }
            }

			if (user->is_phone_user) {
				stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
		            if (dev->sc && !stlc_list_empty(&dev->sc->scene_list)) {
		                stlc_list_for_each_entry(scene_priv_t, scene_priv, &dev->sc->scene_list, link){
		                    if (scene_priv->scene_handle == handle) {
		                        return (void*)scene_priv;
		                    }
		                }
		            }
				}
			}
        }
        break;
		
	case HDLT_TMP:
		stlc_list_for_each_entry(tmp_obj_t, to, &cl_priv->tmp_obj, link) {
			if (to->handle == handle)
				return (void *)to;
		}
		break;
	case HDLT_LINKAGE:
		stlc_list_for_each_entry(la_user_t, puser, &cl_priv->la_user, link) {
			stlc_list_for_each_entry(la_home_t, phome, &puser->home_link, link) {
				if (handle == phome->handle) {
					return (void *)phome;
				}
			}
		}
		break;
	default:
		break;
	}

	log_err(false, "not found object by handle = 0x%08x\n", handle);

	return NULL;
}

user_t *user_lookup_by_name(user_t *user, const char *name)
{
	user_t *dev;
	bool is_sn = cl_is_sn(name);
	u_int64_t sn;

	if (is_sn)
		sn = atoll(name);
	
	stlc_list_for_each_entry(user_t, dev, &user->dev, link) {
		if (is_sn) {
			if (dev->sn == sn)
				return dev;
		} else {
			if ((dev->nickname != NULL && strcmp(dev->nickname, name) == 0) || strcmp(dev->name, name) == 0)
				return dev;
		}
	}

	return NULL;
}

