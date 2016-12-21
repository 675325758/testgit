#include "clib_jni_linkage.h"
#include "cl_linkage.h"

extern jint CopyDevInfo(JNIEnv* env, jclass class_user_info, jobject obj_user_info, cl_user_t *info);

void copy_linkage_rule(JNIEnv *env, jclass class_home, jobject obj_home, cl_la_home_info_t *home) {
	int i = 0;
	jobjectArray ruleArray = NULL;
	jclass class_rule;
	jobject obj_rule;
	jfieldID fid = NULL;

	if (home->rule_num <= 0) {
		return;
	}
	class_rule = (*env)->FindClass(env, CLASS_LINKAGE_RULE);
	ruleArray = (*env)->NewObjectArray(env, home->rule_num, class_rule, NULL);

	for (i = 0; i < home->rule_num; ++i) {
		obj_rule = (*env)->AllocObject(env, class_rule);
		jni_copy_simple_class(env, class_rule, obj_rule,
			                     TRIPLES(int, &home->rule_array[i], rule_id),
			                     TRIPLES(boolean, &home->rule_array[i], enable),
			                     TRIPLES(byte, &home->rule_array[i], state),
			                     TRIPLES(int, &home->rule_array[i], last_exec_time),
			                     TRIPLES(String, &home->rule_array[i], rule),
			                    JNI_VAR_ARG_END);
		(*env)->SetObjectArrayElement(env, ruleArray, i, obj_rule);
		SAFE_DEL_LOCAL_REF(obj_rule);
	}
	fid = (*env)->GetFieldID(env, class_home, "rule_array", "[L" CLASS_LINKAGE_RULE ";");
	(*env)->SetObjectField(env, obj_home, fid, ruleArray);
	SAFE_DEL_LOCAL_REF(ruleArray);
}

static void print_linkage_home(cl_la_home_info_t *home) {
	int i = 0;
	if (home == NULL) {
		return;
	}
	LOGD("xxxddd home info, id = %d,\n", home->home_id);
	LOGD("xxxddd home info, url num = %d, rule num = %d, share num = %d\n", 
		home->url_num, home->rule_num, home->share_desc_num);

	for (i = 0; i < home->url_num; ++i) {
		LOGD("xxxddd home info, url %d = %s\n", i, home->url_array[i]);
	}
	for (i = 0; i < home->rule_num; ++i) {
		LOGD("xxxddd home info, rule %d = %s\n", i, home->rule_array[i].rule);
	}

	for (i = 0; i < home->share_desc_num; ++i) {
		LOGD("xxxddd home info, share %d = %s\n", i, home->share_desc_array[i].desc);
	}
}

void copy_linkage_lable(JNIEnv *env, jclass class_home, jobject obj_home, cl_la_home_info_t *home)
{
	jfieldID fid = NULL;
	jobjectArray lableArray = NULL;
	jobject obj_lable = NULL;
	jclass class_lable = NULL;
	int i = 0;
	
	if (home->label_num <= 0) {
		fid = (*env)->GetFieldID(env, class_home, "lables", "[L"CLASS_LINKAGE_LABLE";");
		(*env)->SetObjectField(env, obj_home, fid, NULL);
		return;
	}

	class_lable = (*env)->FindClass(env, CLASS_LINKAGE_LABLE);
	lableArray = (*env)->NewObjectArray(env, home->label_num, class_lable, NULL);
	fid = (*env)->GetFieldID(env, class_home, "lables", "[L"CLASS_LINKAGE_LABLE";");
	(*env)->SetObjectField(env, obj_home, fid, lableArray);

	for (i = 0; i < home->label_num; ++i) {
		obj_lable = (*env)->AllocObject(env, class_lable);
		jni_copy_simple_class(env, class_lable, obj_lable,
			                     TRIPLES(short, home->label + i, id),
			                     TRIPLES(String, home->label + i, label_name),
			                     QUADRUPLE(long[], home->label + i, sn,  home->label[i].sn_num),
			                     JNI_VAR_ARG_END);
		(*env)->SetObjectArrayElement(env, lableArray, i, obj_lable);
		SAFE_DEL_LOCAL_REF(obj_lable);
	}
	SAFE_DEL_LOCAL_REF(lableArray);
}

void copy_linkage_shortcut(JNIEnv *env, jclass class_home, jobject obj_home, la_sc_key_t *shortcut_key)
{
	JNI_COPY_ARRAY_CLASS(env, class_home, obj_home, CLASS_LINKAGE_SHORTCUT,la_sc_key,
		                   LA_SC_A_NUM, sizeof(la_sc_key_t),
		                   ARRAY_TRIPLES(boolean, shortcut_key, valid),
		                   ARRAY_TRIPLES(String, shortcut_key, name),
		                   ARRAY_TRIPLES(int, shortcut_key, rule_id),
		                   JNI_VAR_ARG_END);
}

void copy_linkage_dict(JNIEnv *env, jclass class_home, jobject obj_home, cl_la_dict_info_t *dict)
{
	jfieldID fid = NULL;
	jobjectArray array_dict = NULL;
	jobject obj_dict = NULL;
	jclass class_dict = NULL;
	int i = 0;

	fid = (*env)->GetFieldID(env, class_home, "dict", "[L"CLASS_LINKAGE_DICT";");
	if (dict == NULL || dict->ndict == 0) {
		(*env)->SetObjectField(env, obj_home, fid, NULL);
		return;
	}

	class_dict = (*env)->FindClass(env, CLASS_LINKAGE_DICT);
	array_dict = (*env)->NewObjectArray(env, dict->ndict, class_dict, NULL);
	(*env)->SetObjectField(env, obj_home, fid, array_dict);
	
	for (i = 0; i < dict->ndict; ++i) {
		obj_dict = (*env)->AllocObject(env, class_dict);
		jni_copy_simple_class(env, class_dict, obj_dict,
			                     QUADRUPLE(byte[], &dict->dict[i], key, dict->dict[i].key_len),
			                     QUADRUPLE(byte[], &dict->dict[i], value, dict->dict[i].value_len),
			                     JNI_VAR_ARG_END);
		(*env)->SetObjectArrayElement(env, array_dict, i, obj_dict);
		SAFE_DEL_LOCAL_REF(obj_dict);
	}

	SAFE_DEL_LOCAL_REF(class_dict);
	SAFE_DEL_LOCAL_REF(array_dict);
}


jobject copy_linkage_home(JNIEnv* env, jclass class_home, cl_user_t *user)
{
	jobject obj_home = NULL;
	cl_la_home_info_t *home = &user->home;
	int i = 0;

	//print_linkage_home(home);
	//LOGD("xxxddd lkrule update home %s, online = %hhu\n", home->home_name, home->online);
	//for (i = 0; i < home->rule_num; ++i) {
	//	LOGD("xxxddd lkrule%d , id = %d, rule = %s\n", i, home->rule_array[i].rule_id, home->rule_array[i].rule);
	//}
	
	obj_home = (*env)->AllocObject(env, class_home);
	jni_copy_simple_class(env, class_home, obj_home,
		                     TRIPLES(int, home, handle),
		                     TRIPLES(boolean, home, online),
		                     TRIPLES(int, home, home_id),
		                     TRIPLES(boolean, home, is_def_home),
		                     TRIPLES(String, home, home_name),
		                     TRIPLES(int, home, last_rule_time),
		                     TRIPLES(int, home, last_template_time),
		                     QUADRUPLE(String[], home, url_array, home->url_num),
		                     TRIPLES(String, home, share),
		                     QUADRUPLE(byte[], home, home_passwd, APP_USER_UUID_NAME_LEN),
		                     JNI_VAR_ARG_END);

	copy_linkage_rule(env, class_home, obj_home, home);
	if ( home->share_desc_num > 0) {
		JNI_COPY_ARRAY_CLASS(env,class_home, obj_home, CLASS_LINKAGE_COMMUNITY_MEMBER, share_desc_array,
			                    home->share_desc_num, sizeof(la_share_desc_t),
			                    ARRAY_TRIPLES(int, home->share_desc_array, user_id),
			                    ARRAY_TRIPLES(int, home->share_desc_array, join_time),
			                    ARRAY_TRIPLES(int, home->share_desc_array, lastuse_time),
			                    ARRAY_TRIPLES(byte, home->share_desc_array, role_id),
			                    ARRAY_TRIPLES(String, home->share_desc_array, desc),
			                    JNI_VAR_ARG_END);
	}
	
	CopyDevInfo(env, class_home, obj_home, user);
	copy_linkage_lable(env, class_home, obj_home, home);
	copy_linkage_shortcut(env, class_home, obj_home, home->la_sc_key);
	//copy_linkage_dict(env, class_home, obj_home, home->dict);
	return obj_home;
}


JNIEXPORT int JNICALL NAME(ClLaAppShareCreate)(JNIEnv* env, jobject this) {
	return cl_la_app_share_create();
}

JNIEXPORT jstring JNICALL NAME(ClLaAppGetShare)(JNIEnv* env, jobject this) {
	char* code = cl_la_app_get_share();
	return (*env)->NewStringUTF(env, !code ? "" : code);
}


JNIEXPORT jobject JNICALL
NAME(ClLkInfoGet)(JNIEnv* env, jobject this)
{
	jclass class_linkage = NULL, class_home = NULL;
	jobject obj_linkage = NULL, obj_home = NULL;
	jobjectArray array_home = NULL;
	jfieldID fid = NULL;
	cl_la_info_t *linkage_info = NULL;
	int i = 0, home_count = 0;;
	//LOGD("xxxddd phone ClLkInfoGet\n");
	linkage_info = cl_la_info_get();
	if (linkage_info == NULL || linkage_info->home_num == 0 || linkage_info->home == NULL) {
		LOGE("xxxddd get linkage info null\n");
		return NULL;
	}
	class_linkage = (*env)->FindClass(env, CLASS_LINKAGE_INFO);
	obj_linkage = (*env)->AllocObject(env, class_linkage);

	jni_copy_simple_class(env, class_linkage, obj_linkage,
		                     QUADRUPLE(int[], linkage_info, user_id, linkage_info->user_num),
		                     TRIPLES(byte, linkage_info, language),
		                     JNI_VAR_ARG_END);
	/*
	LOGD("xxxddd phone num = %hhu\n", linkage_info->phone_num);
	for (i = 0; i < linkage_info->phone_num; ++i) {
		LOGD("xxxddd phone users = %s, cur_index = %hhu\n", linkage_info->phone_name[i], linkage_info->cur_phone);
	}
	*/
	//只有一个用户而已
	if (linkage_info->phone_num > 0 && linkage_info->phone_name != NULL
		&& linkage_info->cur_phone < linkage_info->phone_num	&& linkage_info->cur_phone >= 0) {
		jniCopyString(env, class_linkage, "user_name", obj_linkage, linkage_info->phone_name[linkage_info->cur_phone]);
	}
	//manifest
	/*
	LOGD("xxxddd cap num = %d\n", linkage_info->cap_num);
	for (i = 0; i < linkage_info->cap_num; ++i) {
		LOGD("xxxddd cap url = %s\n", linkage_info->cap_array[i]);
	}
	*/
	//处在能力文件，执行拷贝操作
	if(linkage_info->cap_file.file_type_num > 0){
		for(i=0;i<linkage_info->cap_file.file_type_num;i++){
			cap_file_type_t* cap_file_type = &linkage_info->cap_file.pfile_type[i];
			if(cap_file_type->type == CAP_FILE_TYPE_COM){//一般的能力文件V1
				jniCopyIntValue(env, class_linkage, "manifest_time", obj_linkage, cap_file_type->last_cap_time);
				if (cap_file_type->cap_num> 0 && cap_file_type->cap_array!= NULL) {
					jniCopyString(env, class_linkage, "manifest_url", obj_linkage, cap_file_type->cap_array[0]);
				}
			}else if(cap_file_type->type == CAP_FILE_TYPE_CUSTOM){//自定义联动能力文件V2
				jniCopyIntValue(env, class_linkage, "custom_manifest_time", obj_linkage, cap_file_type->last_cap_time);
				if (cap_file_type->cap_num> 0 && cap_file_type->cap_array!= NULL) {
					jniCopyString(env, class_linkage, "custom_manifest_url", obj_linkage, cap_file_type->cap_array[0]);
				}
			}else {
				LOGE("xxxddd invalid cap file,ignore it . type = %d\n", cap_file_type->type);
			}
		}
	}

	/*
	jniCopyIntValue(env, class_linkage, "manifest_time", obj_linkage, linkage_info->last_cap_time);
	if (linkage_info->cap_num > 0 && linkage_info->cap_array != NULL) {
		jniCopyString(env, class_linkage, "manifest_url", obj_linkage, linkage_info->cap_array[0]);
	}
	*/
	
	for (i = 0; i < linkage_info->home_num; ++i) {
		if (linkage_info->home[i]->is_la) {
			++home_count;
		}
	}
	//LOGD("xxxddd home count = %d, clib count = %d\n", home_count, linkage_info->home_num);
	class_home = (*env)->FindClass(env, CLASS_LINKAGE_COMMUNITY);
	array_home = (*env)->NewObjectArray(env, home_count, class_home, NULL);
	home_count = 0;
	for (i = 0; i < linkage_info->home_num; ++i) {
		if (linkage_info->home[i]->is_la) {
			obj_home = copy_linkage_home(env, class_home, linkage_info->home[i]);
			(*env)->SetObjectArrayElement(env, array_home, home_count, obj_home);
			++home_count;
			SAFE_DEL_LOCAL_REF(obj_home);
		}
	}

	fid = (*env)->GetFieldID(env, class_linkage, "communities", "[L" CLASS_LINKAGE_COMMUNITY ";");
	(*env)->SetObjectField(env, obj_linkage, fid, array_home);

	SAFE_DEL_LOCAL_REF(class_linkage);
	SAFE_DEL_LOCAL_REF(class_home);
	SAFE_DEL_LOCAL_REF(array_home);
	cl_la_info_free(linkage_info);
	return obj_linkage;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityCreate)(JNIEnv* env, jobject this, jobject jhome, jstring name)
{
	jclass class_home = NULL;
	jobject obj_home = NULL;
	jfieldID fid = NULL;
	cl_handle_t handle = 0;
	const char * c_name = NULL;

	c_name = (*env)->GetStringUTFChars(env, name, NULL);
	int ret = cl_la_home_create(&handle, (char *)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);

	class_home = (*env)->GetObjectClass(env, jhome);
	fid = (*env)->GetFieldID(env, class_home, "handle", "I");
	(*env)->SetIntField(env, jhome, fid, handle);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityDel)(JNIEnv* env, jobject this, jint home_handle, jint home_id)
{
	return cl_la_home_del(home_handle, home_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityAddDev)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jlongArray sn)
{
	jlong *c_sn = NULL;
	u_int8_t num = 0;
	int ret = 0;

	num = (u_int8_t)(*env)->GetArrayLength(env, sn);
	c_sn = (*env)->GetLongArrayElements(env, sn, NULL);
	
	ret = cl_la_home_adddev(home_handle, home_id, num, (u_int64_t*)c_sn);
	(*env)->ReleaseLongArrayElements(env, sn, c_sn, JNI_ABORT);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityDelDev)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jlongArray sn, jbyte flag)
{
	jlong *c_sn = NULL;
	u_int8_t num = 0;
	int ret = 0;

	if (sn == NULL) {
		return RS_INVALID_PARAM;
	}
	
	num = (u_int8_t)(*env)->GetArrayLength(env, sn);
	c_sn = (*env)->GetLongArrayElements(env, sn, NULL);
	
	ret = cl_la_home_removedev(home_handle, home_id, num, (u_int64_t*)c_sn, flag);
	(*env)->ReleaseLongArrayElements(env, sn, c_sn, JNI_ABORT);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityModulesQuery)(JNIEnv* env, jobject this, jint home_handle, jint home_id)
{
	return cl_la_config_template_query(home_handle, home_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityRuleQuery)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint rule_id)
{
	return cl_la_config_rule_query(home_handle, home_id, rule_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityRuleAdd)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint rule_id, jstring rule, jboolean enable)
{
	const char * c_rule = (*env)->GetStringUTFChars(env, rule, NULL);
	//LOGD("xxxddd set rule len = %d\n", strlen(c_rule));
	int ret = cl_la_config_rule_add(home_handle, home_id, rule_id, (char *)c_rule, enable);
	(*env)->ReleaseStringUTFChars(env, rule, c_rule);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityRuleDel)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint rule_id)
{
	return cl_la_config_rule_del(home_handle, home_id, rule_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityRuleEnable)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint rule_id, jboolean enable)
{
	return cl_la_config_rule_modify(home_handle, home_id, rule_id, enable);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityRuleExec)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint rule_id)
{
	return cl_la_config_rule_exec(home_handle, home_id, rule_id);
}
JNIEXPORT jint JNICALL
NAME(ClLkHomeRuleExec)(JNIEnv* env, jobject this, jint home_id, jstring rule_json)
{
	if (rule_json == NULL) {
		LOGE("ClLkHomeRuleExec json string is null!!");
		return RS_INVALID_PARAM;
	}
	
	const char * c_rule = (*env)->GetStringUTFChars(env, rule_json, NULL);
	int ret = cl_la_home_rule_excute(home_id, (char *) c_rule);

	(*env)->ReleaseStringUTFChars(env, rule_json, c_rule);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityConfigEvent)(JNIEnv* env, jobject this, jint home_handle, jint event, jint home_id, jint rule_id)
{
	return cl_la_config_event(home_handle, event, home_id, rule_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityShareRequest)(JNIEnv* env, jobject this, jint home_handle, jint home_id)
{
	return cl_la_home_share_create(home_handle, home_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityShareRegister)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jstring share_code, jstring desc)
{
	const char * c_desc = (*env)->GetStringUTFChars(env, desc, NULL);
	const char *c_share_code = (*env)->GetStringUTFChars(env, share_code, NULL);;
	int ret = 0;

	ret = cl_la_home_share_register(home_handle, home_id, (char *)c_share_code, (char *)c_desc);
	(*env)->ReleaseStringUTFChars(env, desc, c_desc);
	(*env)->ReleaseStringUTFChars(env, share_code, c_share_code);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityShareQuery)(JNIEnv* env, jobject this, jint home_handle, jint home_id)
{
	return cl_la_home_share_query(home_handle, home_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityShareEdit)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint user_id, jbyte role_id, jstring desc)
{
	const char * c_desc = (*env)->GetStringUTFChars(env, desc, NULL);
	int ret = cl_la_home_share_edit(home_handle, home_id, user_id, role_id, (char *)c_desc);
	(*env)->ReleaseStringUTFChars(env, desc, c_desc);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityShareDel)(JNIEnv* env, jobject this, jint home_handle, jint home_id, jint user_id)
{
	return cl_la_home_share_del(home_handle, home_id, user_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunitySetActive)(JNIEnv* env, jobject this,jint home_id)
{
	return cl_la_cur_homeid_set(home_id);
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityEdit)(JNIEnv* env, jobject this,jint home_handle, jint home_id, jstring name)
{
	const char * c_name = (*env)->GetStringUTFChars(env, name, NULL);
	int ret = cl_la_home_name_modify(home_handle, home_id, (char *)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	return ret;
}


JNIEXPORT jint JNICALL
NAME(ClLkCommunityMoveDev)(JNIEnv* env, jobject this, jint home_handle, jint src_home_id, jint dst_home_id, jlongArray sn)
{
	jlong *c_sn = NULL;
	u_int8_t num = 0;
	int ret = 0;

	if (sn == NULL) {
		return RS_INVALID_PARAM;
	}
	
	num = (u_int8_t)(*env)->GetArrayLength(env, sn);
	c_sn = (*env)->GetLongArrayElements(env, sn, NULL);
	
	ret = cl_la_home_movedev(home_handle, src_home_id, dst_home_id, num, (u_int64_t*)c_sn);
	(*env)->ReleaseLongArrayElements(env, sn, c_sn, JNI_ABORT);
	return ret;
}



JNIEXPORT jint JNICALL
NAME(ClLkCommunityAutoLogin)(JNIEnv* env, jobject this, jint home_handle, jlongArray sn)
{
	jlong *c_sn = NULL;
	u_int16_t num = 0;
	int ret = 0; 

	num = (u_int16_t)(*env)->GetArrayLength(env, sn);
	c_sn = (*env)->GetLongArrayElements(env, sn, NULL);
	
	ret = cl_la_sdk_login_set(home_handle, (u_int64_t*)sn, num);
	(*env)->ReleaseLongArrayElements(env, sn, c_sn, JNI_ABORT);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserCreate)(JNIEnv* env, jobject this,jstring name, jstring passwd)
{
	//LOGD("use create in\n");
	if (name == NULL || passwd == NULL) {
		return RS_INVALID_PARAM;
	}
	const char *c_name = (*env)->GetStringUTFChars(env, name, NULL);
	const char *c_passwd = (*env)->GetStringUTFChars(env, passwd, NULL);
	//LOGD("use create , name = %s, pwd = %s\n", c_name, c_passwd);
	int ret = cl_la_phone_create((char *)c_name, (char *)c_passwd);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	(*env)->ReleaseStringUTFChars(env, passwd, c_passwd);
	//LOGD("use create out ret = %d\n", ret);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserLogin)(JNIEnv* env, jobject this,jstring name, jstring passwd)
{
	if (name == NULL || passwd == NULL) {
		return RS_INVALID_PARAM;
	}
	const char *c_name = (*env)->GetStringUTFChars(env, name, NULL);
	const char *c_passwd = (*env)->GetStringUTFChars(env, passwd, NULL);
	int ret =  cl_la_phone_login((char *)c_name, (char *)c_passwd);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	(*env)->ReleaseStringUTFChars(env, passwd, c_passwd);
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserLogout)(JNIEnv* env, jobject this,jstring name)
{
	const char *c_name = (*env)->GetStringUTFChars(env, name, NULL);
	int ret =  cl_la_phone_logout((char *)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserDel)(JNIEnv* env, jobject this,jstring name)
{
	const char *c_name = (*env)->GetStringUTFChars(env, name, NULL);
	int ret = cl_la_phone_del((char *)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserSwitch)(JNIEnv* env, jobject this,jstring name)
{
	const char *c_name = (*env)->GetStringUTFChars(env, name, NULL);
	int ret =  cl_la_phone_swich((char *)c_name);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserModify)(JNIEnv* env, jobject this,jstring name, jstring passwd)
{
	if (name == NULL || passwd == NULL) {
		return RS_INVALID_PARAM;
	}
	const char *c_name = (*env)->GetStringUTFChars(env, name, NULL);
	const char *c_passwd = (*env)->GetStringUTFChars(env, passwd, NULL);
	int ret =  cl_la_phone_passwd_modify((char *)c_name, (char *)c_passwd);
	(*env)->ReleaseStringUTFChars(env, name, c_name);
	(*env)->ReleaseStringUTFChars(env, passwd, c_passwd);
	return ret;
}

JNIEXPORT jobject JNICALL
NAME(ClLkGetNotify)(JNIEnv* env, jobject this)
{
	cl_la_notify_msg_t *lk_msg = NULL;
	jclass class_msg = NULL;
	jobject obj_msg = NULL;

	lk_msg = cl_la_get_notify_msg();
	if (lk_msg == NULL) {
		return NULL;
	}

	class_msg = (*env)->FindClass(env, CLASS_LINKAGE_NOTIFY);
	obj_msg = (*env)->AllocObject(env, class_msg);

	jni_copy_simple_class(env, class_msg, obj_msg,
		                               TRIPLES(short, lk_msg, type),
		                               TRIPLES(String, lk_msg, cn_msg),
		                               TRIPLES(String, lk_msg, en_msg),
		                               JNI_VAR_ARG_END);
	cl_la_free_notify_msg(lk_msg);
	SAFE_DEL_LOCAL_REF(class_msg);
	return obj_msg;
}



JNIEXPORT jint JNICALL
NAME(ClLkCommunityConfClean)(JNIEnv* env, jobject this)
{
	cl_la_conf_clean();
	return RS_OK;
}


JNIEXPORT jobject JNICALL
NAME(ClLkCommunityUpdate)(JNIEnv* env, jobject this, jint home_handle)
{
	cl_la_info_t *lk_info = cl_la_info_get();
	cl_user_t *home = NULL;
	jclass class_home = NULL;
	jobject obj_home = NULL;
	int i = 0;

	if (lk_info == NULL) {
		LOGE("xxxddd ClLkCommunityUpdate null\n");
		return NULL;
	}
	for (i = 0; i < lk_info->home_num; ++i) {
		if (lk_info->home[i]->home.handle == home_handle) {
			home = lk_info->home[i];
			break;
		}
	}
	if (home == NULL) {
		return NULL;
	}
	class_home = (*env)->FindClass(env, CLASS_LINKAGE_COMMUNITY);
	obj_home = copy_linkage_home(env, class_home, home);
	cl_la_info_free(lk_info);
	SAFE_DEL_LOCAL_REF(class_home);

	return obj_home;
}

JNIEXPORT jint JNICALL
NAME(ClLkUserGetName)(JNIEnv* env, jobject this, jobject obj_info)
{
	cl_la_info_t *lk_info = cl_la_info_get();
	jfieldID fid;
	jstring user_name = NULL;
	jclass class_info = NULL;
	int i = 0;

	if (lk_info == NULL || lk_info->phone_name == NULL 
		|| lk_info->phone_num == 0 || lk_info->cur_phone >= lk_info->phone_num) {
		return RS_INVALID_PARAM;
	}
	/*
	LOGD("xxxddd phone num = %hhu\n", lk_info->phone_num);
	for (i = 0; i < lk_info->phone_num; ++i) {
		LOGD("xxxddd phone users = %s, cur_index = %hhu\n", lk_info->phone_name[i], lk_info->cur_phone);
	}
	*/

	class_info = (*env)->GetObjectClass(env, obj_info);
	jniCopyString(env, class_info, "user_name", obj_info, lk_info->phone_name[lk_info->cur_phone]);
	jniCopyIntArray(env, class_info, "user_id", obj_info, lk_info->user_id, lk_info->user_num);
	//LOGD("ClLkUserGetName quit, name = %s\n", lk_info->phone_name[lk_info->cur_phone]);
	cl_la_info_free(lk_info);
	return RS_OK;
}

JNIEXPORT jint JNICALL
NAME(ClLkSetLanguage)(JNIEnv* env, jobject this, jbyte lang)
{
	return cl_la_cur_lang_set(lang);
}

JNIEXPORT jint JNICALL
NAME(ClLkGetLanguage)(JNIEnv* env, jobject this)
{
	u_int8_t lang = 0;
	int ret = cl_la_cur_lang_get(&lang);
	if (ret == RS_OK) {
		ret = lang;
	}
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkManifestQuery)(JNIEnv* env, jobject this)
{
	return cl_la_cap_file_query();
}

JNIEXPORT jint JNICALL
NAME(ClLkUpdateManifest)(JNIEnv* env, jobject this, jobject obj_linkage)
{
	jclass class_linkage = NULL;
	cl_la_info_t *linkage_info = NULL;
	int i = 0;

	linkage_info  = cl_la_info_get();
	if (linkage_info == NULL) {
		LOGE("xxxddd ClLkUpdateManifest, linkage_info is NULL\n");
		return RS_ERROR;
	}
	class_linkage = (*env)->GetObjectClass(env, obj_linkage);
	/*
	LOGD("xxxddd cap num = %d\n", linkage_info->cap_num);
	for (i = 0; i < linkage_info->cap_num; ++i) {
		LOGD("xxxddd cap url = %s\n", linkage_info->cap_array[i]);
	}
	*/

	//处在能力文件，执行拷贝操作
	if(linkage_info->cap_file.file_type_num > 0){
		for(i=0;i<linkage_info->cap_file.file_type_num;i++){
			cap_file_type_t* cap_file_type = &linkage_info->cap_file.pfile_type[i];
			if(cap_file_type->type == CAP_FILE_TYPE_COM){//一般的能力文件V1
				jniCopyIntValue(env, class_linkage, "manifest_time", obj_linkage, cap_file_type->last_cap_time);
				if (cap_file_type->cap_num> 0 && cap_file_type->cap_array!= NULL) {
					jniCopyString(env, class_linkage, "manifest_url", obj_linkage, cap_file_type->cap_array[0]);
				}
			}else if(cap_file_type->type == CAP_FILE_TYPE_CUSTOM){//自定义联动能力文件V2
				jniCopyIntValue(env, class_linkage, "custom_manifest_time", obj_linkage, cap_file_type->last_cap_time);
				if (cap_file_type->cap_num> 0 && cap_file_type->cap_array!= NULL) {
					jniCopyString(env, class_linkage, "custom_manifest_url", obj_linkage, cap_file_type->cap_array[0]);
				}
			}else {
				LOGE("xxxddd invalid cap file,ignore it . type = %d\n", cap_file_type->type);
			}
		}
	}

	/*jniCopyIntValue(env, class_linkage, "manifest_time", obj_linkage, linkage_info->last_cap_time);
	if (linkage_info->cap_num > 0 && linkage_info->cap_array != NULL) {
		jniCopyString(env, class_linkage, "manifest_url", obj_linkage, linkage_info->cap_array[0]);
	}*/
	
	cl_la_info_free(linkage_info);
	SAFE_DEL_LOCAL_REF(class_linkage);
	return RS_OK;
}

JNIEXPORT jboolean JNICALL
NAME(ClLkIsSameUserPwd)(JNIEnv* env, jobject this, jstring username, jstring newPwd)
{
	if (username == NULL || newPwd == NULL) {
		return RS_INVALID_PARAM;
	}
	const char *c_name = (*env)->GetStringUTFChars(env, username, NULL);
	const char *c_passwd = (*env)->GetStringUTFChars(env, newPwd, NULL);
	bool ret =  cl_la_phone_is_same_passwd((char *)c_name, (char *)c_passwd);
	(*env)->ReleaseStringUTFChars(env, username, c_name);
	(*env)->ReleaseStringUTFChars(env, newPwd, c_passwd);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLaLabelSet)(JNIEnv* env, jobject this, jint home_id, jshort lable_id, jstring label_name, jlongArray sn)
{
	u_int32_t num = 0;
	char *c_name = NULL;
	u_int64_t *c_sn = NULL;
	int ret = RS_OK;

	if (sn != NULL) {
		num = (int)(*env)->GetArrayLength(env, sn);
		c_sn = (u_int64_t *)(*env)->GetLongArrayElements(env, sn, NULL);
	}
	
	c_name = (char *)(*env)->GetStringUTFChars(env, label_name, NULL);

	ret = cl_la_label_add_modify(home_id, (u_int8_t)lable_id, c_name, num, c_sn);

	if (c_sn != NULL) {
		(*env)->ReleaseLongArrayElements(env, sn, (jlong *)c_sn, JNI_ABORT);
	}
	(*env)->ReleaseStringUTFChars(env, label_name, c_name);
	
	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLaLabelDel)(JNIEnv* env, jobject this, jint home_id, jshort lable_id)
{
	return cl_la_label_del(home_id, (u_int8_t)lable_id);
}

JNIEXPORT jint JNICALL
NAME(ClLaBindDevToLabel)(JNIEnv* env, jobject this, jint home_id, jlong dev_sn, jshortArray lable_ids)
{
	u_int32_t num = 0;
	short *c_ids = NULL;
	int ret = RS_OK; 
	int i = 0;

	if (lable_ids == NULL) {
		return cl_la_bind_devs_to_label(home_id, dev_sn, 0, NULL);
	}

	num = (int)(*env)->GetArrayLength(env, lable_ids);
	if (num == 0) {
		return cl_la_bind_devs_to_label(home_id, dev_sn, 0, NULL);;
	}
	
	c_ids = (*env)->GetShortArrayElements(env, lable_ids, NULL);
	if (c_ids == NULL) {
		return cl_la_bind_devs_to_label(home_id, dev_sn, 0, NULL);;
	}
	
	ret = cl_la_bind_devs_to_label(home_id, dev_sn, num, c_ids);
	(*env)->ReleaseShortArrayElements(env, lable_ids, c_ids, JNI_ABORT);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLaShortcutSet)(JNIEnv* env, jobject this, jint home_id, jbyte index, jboolean enable, jstring name, jint rule_id, jstring rule)
{
	const char *c_name = NULL;
	const char *c_rule = NULL;
	int ret = RS_OK;

	c_name = (char *)(*env)->GetStringUTFChars(env, name, NULL);
	c_rule = (char *)(*env)->GetStringUTFChars(env, rule, NULL);

	ret = cl_la_shortcut_rule_bind(home_id, index, enable, (char *)c_name, rule_id, (char *)c_rule);

	(*env)->ReleaseStringUTFChars(env, name, c_name);
	(*env)->ReleaseStringUTFChars(env, rule, c_rule);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLaDictSet)(JNIEnv* env, jobject this, jint home_id, jbyteArray key, jbyteArray value)
{
	u_int8_t *c_key = NULL;
	u_int8_t *c_value = NULL;
	u_int16_t c_key_len = 0;
	u_int16_t c_value_len = 0;
	int ret = 0;

	if (key == NULL || value == NULL) {
		return RS_INVALID_PARAM;
	}
	c_key_len = (u_int16_t)(*env)->GetArrayLength(env, key);
	c_value_len = (u_int16_t)(*env)->GetArrayLength(env, value);
	if (c_key_len == 0 || c_value_len == 0) {
		return RS_INVALID_PARAM;
	}
	c_key = (*env)->GetByteArrayElements(env, key, NULL);
	c_value = (*env)->GetByteArrayElements(env, value, NULL);

	ret = cl_la_dict_set(home_id, c_key, c_value, c_key_len, c_value_len);
	(*env)->ReleaseByteArrayElements(env, key, c_key, JNI_ABORT);
	(*env)->ReleaseByteArrayElements(env, value, c_value, JNI_ABORT);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLaDictDel)(JNIEnv* env, jobject this, jint home_id, jbyteArray key)
{
	u_int8_t *c_key = NULL;
	u_int16_t c_key_len = 0;
	int ret = 0;

	if (key == NULL) {
		return RS_INVALID_PARAM;
	}
	c_key_len = (u_int16_t)(*env)->GetArrayLength(env, key);
	if (c_key_len == 0) {
		return RS_INVALID_PARAM;
	}
	c_key = (*env)->GetByteArrayElements(env, key, NULL);

	ret = cl_la_dict_del(home_id, c_key, c_key_len);
	(*env)->ReleaseByteArrayElements(env, key, c_key, JNI_ABORT);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLaDictQuery)(JNIEnv* env, jobject this, jint home_id, jbyteArray key)
{
	u_int8_t *c_key = NULL;
	u_int16_t c_key_len = 0;
	int ret = 0;

	if (key == NULL) {
		return RS_INVALID_PARAM;
	}
	c_key_len = (u_int16_t)(*env)->GetArrayLength(env, key);
	if (c_key_len == 0) {
		return RS_INVALID_PARAM;
	}
	c_key = (*env)->GetByteArrayElements(env, key, NULL);

	ret = cl_la_dict_query(home_id, c_key, c_key_len);
	(*env)->ReleaseByteArrayElements(env, key, c_key, JNI_ABORT);

	return ret;
}

JNIEXPORT jint JNICALL
NAME(ClLkCommunityRuleLogQuery)(JNIEnv* env, jobject this, jint home_id, jint index, jint count)
{
	return cl_la_home_rules_logs_query(home_id, index, count);
}


JNIEXPORT jobject JNICALL
NAME(ClLkCommunityRuleLogGet)(JNIEnv* env, jobject this, jint home_id)
{
	cl_home_log_rule_change_t *rule_logs = NULL;
	jclass class_log_item = NULL;
	jobjectArray array_log_item = NULL;
	jobject obj_log_item = NULL;
	jclass class_log = NULL;
	jobject obj_log = NULL;
	jfieldID fid = NULL;
	int i = 0;
	

	rule_logs = cl_la_home_rules_logs_get(home_id);
	if (rule_logs == NULL || rule_logs->home_id != home_id) {
		return NULL;
	}
	class_log = (*env)->FindClass(env, CLASS_LINKAGE_RULE_LOG);
	obj_log = (*env)->AllocObject(env, class_log);

	class_log_item = (*env)->FindClass(env, CLASS_LINKAGE_RULE_LOG_ITEM);
	jniCopyIntValue(env, class_log, "latestIndex", obj_log, rule_logs->max_index);
	if (rule_logs->log_count > 0) {
		array_log_item = (*env)->NewObjectArray(env, rule_logs->log_count, class_log_item, NULL);
		for (i = 0; i < rule_logs->log_count; ++i) {
			obj_log_item = (*env)->AllocObject(env, class_log_item);
			jni_copy_simple_class(env, class_log_item, obj_log_item,
									 TRIPLES(int, &rule_logs->plog[i], index),
				                     TRIPLES(boolean, &rule_logs->plog[i], is_test),
				                     TRIPLES(boolean, &rule_logs->plog[i], is_timer),
				                     QUADRUPLE(long[], &rule_logs->plog[i], pmonitor_sn, rule_logs->plog[i].monitor_count),
					                 QUADRUPLE(long[], &rule_logs->plog[i], paction_sn, rule_logs->plog[i].action_count),
				                     TRIPLES(int, &rule_logs->plog[i], ruld_id),
			                         TRIPLES(int, &rule_logs->plog[i], time_stamp),
			                         JNI_VAR_ARG_END);
			(*env)->SetObjectArrayElement(env, array_log_item, i, obj_log_item);
			SAFE_DEL_LOCAL_REF(obj_log_item);
		}
		fid = (*env)->GetFieldID(env, class_log, "plog", "[L"CLASS_LINKAGE_RULE_LOG_ITEM";");
		(*env)->SetObjectField(env, obj_log, fid, array_log_item);
	}
	

	SAFE_DEL_LOCAL_REF(class_log);
	SAFE_DEL_LOCAL_REF(array_log_item);
	SAFE_DEL_LOCAL_REF(class_log_item);
	cl_la_home_rules_logs_free(rule_logs);

	return obj_log;
}



