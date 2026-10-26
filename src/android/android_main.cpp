#include "android_main.h"

#include <base/fs.h>
#include <base/hash.h>
#include <base/io.h>
#include <base/log.h>
#include <base/str.h>

#include <engine/shared/linereader.h>

#include <SDL.h>
#include <jni.h>

#include <algorithm>
#include <string>
#include <vector>

static bool UnpackAsset(const char *pFilename)
{
	char aAssetFilename[IO_MAX_PATH_LENGTH] = "asset_integrity_files/";
	str_append(aAssetFilename, pFilename);

	// This uses SDL_RWFromFile because it can read Android assets,
	// which are files stored in the app's APK file. All data files
	// are stored as assets and unpacked to the external storage.
	SDL_RWops *pAssetFile = SDL_RWFromFile(aAssetFilename, "rb");
	if(!pAssetFile)
	{
		log_error("android", "Failed to open asset '%s' for reading", pFilename);
		return false;
	}

	const long int FileLength = SDL_RWsize(pAssetFile);
	if(FileLength < 0)
	{
		SDL_RWclose(pAssetFile);
		log_error("android", "Failed to determine length of asset '%s'", pFilename);
		return false;
	}

	char *pData = static_cast<char *>(malloc(FileLength));
	const size_t ReadLength = SDL_RWread(pAssetFile, pData, 1, FileLength);
	SDL_RWclose(pAssetFile);

	if(ReadLength != (size_t)FileLength)
	{
		free(pData);
		log_error("android", "Failed to read asset '%s' (read %" PRIzu ", wanted %ld)", pFilename, ReadLength, FileLength);
		return false;
	}

	IOHANDLE TargetFile = io_open(pFilename, IOFLAG_WRITE);
	if(!TargetFile)
	{
		free(pData);
		log_error("android", "Failed to open '%s' for writing", pFilename);
		return false;
	}

	const size_t WriteLength = io_write(TargetFile, pData, FileLength);
	io_close(TargetFile);
	free(pData);

	if(WriteLength != (size_t)FileLength)
	{
		log_error("android", "Failed to write data to '%s' (wrote %" PRIzu ", wanted %ld)", pFilename, WriteLength, FileLength);
		return false;
	}

	return true;
}

constexpr const char *INTEGRITY_INDEX = "integrity.txt";
constexpr const char *INTEGRITY_INDEX_SAVE = "integrity_save.txt";

// The first line of each integrity file contains the combined hash for all files,
// if the hashes match then we assume that the unpacked data folder is up-to-date.
static bool EqualIntegrityFiles(const char *pAssetFilename, const char *pStorageFilename)
{
	IOHANDLE StorageFile = io_open(pStorageFilename, IOFLAG_READ);
	if(!StorageFile)
	{
		return false;
	}

	char aStorageMainSha256[SHA256_MAXSTRSIZE];
	const size_t StorageReadLength = io_read(StorageFile, aStorageMainSha256, sizeof(aStorageMainSha256) - 1);
	io_close(StorageFile);
	if(StorageReadLength != sizeof(aStorageMainSha256) - 1)
	{
		return false;
	}
	aStorageMainSha256[sizeof(aStorageMainSha256) - 1] = '\0';

	char aAssetFilename[IO_MAX_PATH_LENGTH] = "asset_integrity_files/";
	str_append(aAssetFilename, pAssetFilename);

	SDL_RWops *pAssetFile = SDL_RWFromFile(aAssetFilename, "rb");
	if(!pAssetFile)
	{
		return false;
	}

	char aAssetMainSha256[SHA256_MAXSTRSIZE];
	const size_t AssetReadLength = SDL_RWread(pAssetFile, aAssetMainSha256, 1, sizeof(aAssetMainSha256) - 1);
	SDL_RWclose(pAssetFile);
	if(AssetReadLength != sizeof(aAssetMainSha256) - 1)
	{
		return false;
	}
	aAssetMainSha256[sizeof(aAssetMainSha256) - 1] = '\0';

	return str_comp(aStorageMainSha256, aAssetMainSha256) == 0;
}

class CIntegrityFileLine
{
public:
	char m_aFilename[IO_MAX_PATH_LENGTH];
	SHA256_DIGEST m_Sha256;
};

static std::vector<CIntegrityFileLine> ReadIntegrityFile(const char *pFilename)
{
	CLineReader LineReader;
	if(!LineReader.OpenFile(io_open(pFilename, IOFLAG_READ)))
	{
		return {};
	}

	std::vector<CIntegrityFileLine> vLines;
	while(const char *pReadLine = LineReader.Get())
	{
		const char *pSpaceInLine = str_rchr(pReadLine, ' ');
		CIntegrityFileLine Line;
		char aSha256[SHA256_MAXSTRSIZE];
		if(pSpaceInLine == nullptr)
		{
			if(!vLines.empty())
			{
				// Only the first line is allowed to not contain a filename
				log_error("android", "Failed to parse line %" PRIzu " of '%s': line does not contain space", vLines.size() + 1, pFilename);
				return {};
			}
			Line.m_aFilename[0] = '\0';
			str_copy(aSha256, pReadLine);
		}
		else
		{
			str_truncate(Line.m_aFilename, sizeof(Line.m_aFilename), pReadLine, pSpaceInLine - pReadLine);
			str_copy(aSha256, pSpaceInLine + 1);
		}
		if(sha256_from_str(&Line.m_Sha256, aSha256) != 0)
		{
			log_error("android", "Failed to parse line %" PRIzu " of '%s': invalid SHA256 string", vLines.size() + 1, pFilename);
			return {};
		}
		vLines.emplace_back(std::move(Line));
	}

	return vLines;
}

// bestclient-light
static bool CopyFileRaw(const char *pSrc, const char *pDst)
{
	IOHANDLE Src = io_open(pSrc, IOFLAG_READ);
	if(!Src)
		return false;
	IOHANDLE Dst = io_open(pDst, IOFLAG_WRITE);
	if(!Dst)
	{
		io_close(Src);
		return false;
	}
	unsigned char aBuf[64 * 1024];
	while(true)
	{
		const unsigned Read = io_read(Src, aBuf, sizeof(aBuf));
		if(Read == 0)
			break;
		if(io_write(Dst, aBuf, Read) != Read)
		{
			io_close(Src);
			io_close(Dst);
			return false;
		}
	}
	io_close(Src);
	io_close(Dst);
	return true;
}

struct SCopyTree
{
	char m_aFrom[IO_MAX_PATH_LENGTH];
	char m_aTo[IO_MAX_PATH_LENGTH];
};

static int CopyTreeEntry(const char *pName, int IsDir, int DirType, void *pUser)
{
	(void)DirType;
	if(pName == nullptr || pName[0] == '\0' || str_comp(pName, ".") == 0 || str_comp(pName, "..") == 0)
		return 0;
	SCopyTree *pTree = static_cast<SCopyTree *>(pUser);
	char aSrc[IO_MAX_PATH_LENGTH];
	char aDst[IO_MAX_PATH_LENGTH];
	str_format(aSrc, sizeof(aSrc), "%s/%s", pTree->m_aFrom, pName);
	str_format(aDst, sizeof(aDst), "%s/%s", pTree->m_aTo, pName);
	if(IsDir)
	{
		fs_makedir(aDst);
		SCopyTree Child;
		str_copy(Child.m_aFrom, aSrc);
		str_copy(Child.m_aTo, aDst);
		fs_listdir(aSrc, CopyTreeEntry, 0, &Child);
	}
	else
	{
		CopyFileRaw(aSrc, aDst);
	}
	return 0;
}

static bool AndroidCallString(const char *pMethodName, const char *pSignature, const char *pArgument, char *pOut, int OutSize)
{
	JNIEnv *pEnv = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
	if(pEnv == nullptr)
		return false;
	jobject Activity = (jobject)SDL_AndroidGetActivity();
	if(Activity == nullptr)
		return false;
	jclass ActivityClass = pEnv->GetObjectClass(Activity);
	jmethodID MethodId = pEnv->GetMethodID(ActivityClass, pMethodName, pSignature);
	bool Ok = false;
	if(MethodId != nullptr)
	{
		jstring Argument = pArgument != nullptr ? pEnv->NewStringUTF(pArgument) : nullptr;
		jstring Result = nullptr;
		if(pArgument != nullptr)
			Result = (jstring)pEnv->CallObjectMethod(Activity, MethodId, Argument);
		else
			Result = (jstring)pEnv->CallObjectMethod(Activity, MethodId);
		if(pEnv->ExceptionCheck())
		{
			pEnv->ExceptionClear();
			Result = nullptr;
		}
		if(Result != nullptr)
		{
			const char *pUtf = pEnv->GetStringUTFChars(Result, nullptr);
			if(pUtf != nullptr)
			{
				str_copy(pOut, pUtf, OutSize);
				pEnv->ReleaseStringUTFChars(Result, pUtf);
				Ok = pOut[0] != '\0';
			}
			pEnv->DeleteLocalRef(Result);
		}
		if(Argument != nullptr)
			pEnv->DeleteLocalRef(Argument);
	}
	else if(pEnv->ExceptionCheck())
	{
		pEnv->ExceptionClear();
	}
	pEnv->DeleteLocalRef(ActivityClass);
	pEnv->DeleteLocalRef(Activity);
	return Ok;
}

static bool AndroidPersistentRoot(char *pPath, int PathSize)
{
	pPath[0] = '\0';
	return AndroidCallString("persistentStoragePath", "()Ljava/lang/String;", nullptr, pPath, PathSize);
}

bool OpenAndroidFolder(const char *pPath)
{
	if(pPath == nullptr || pPath[0] == '\0')
		return false;
	JNIEnv *pEnv = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
	if(pEnv == nullptr)
		return false;
	jobject Activity = (jobject)SDL_AndroidGetActivity();
	if(Activity == nullptr)
		return false;
	jclass ActivityClass = pEnv->GetObjectClass(Activity);
	jmethodID MethodId = pEnv->GetMethodID(ActivityClass, "openFolder", "(Ljava/lang/String;)Z");
	bool Ok = false;
	if(MethodId != nullptr)
	{
		jstring Path = pEnv->NewStringUTF(pPath);
		Ok = pEnv->CallBooleanMethod(Activity, MethodId, Path);
		if(pEnv->ExceptionCheck())
		{
			pEnv->ExceptionClear();
			Ok = false;
		}
		pEnv->DeleteLocalRef(Path);
	}
	else if(pEnv->ExceptionCheck())
	{
		pEnv->ExceptionClear();
	}
	pEnv->DeleteLocalRef(ActivityClass);
	pEnv->DeleteLocalRef(Activity);
	return Ok;
}

static bool AndroidUserDirHasConfig(const char *pUserDir)
{
	char aPath[IO_MAX_PATH_LENGTH];
	str_format(aPath, sizeof(aPath), "%s/settings_ddnet.cfg", pUserDir);
	if(fs_is_file(aPath))
		return true;
	str_format(aPath, sizeof(aPath), "%s/settings_BestClient-Light.cfg", pUserDir);
	return fs_is_file(aPath);
}

static void MigratePrivateUserDir(const char *pPrivateRoot)
{
	if(pPrivateRoot == nullptr || pPrivateRoot[0] == '\0')
		return;
	if(AndroidUserDirHasConfig("user"))
		return;
	char aPrivateUser[IO_MAX_PATH_LENGTH];
	str_format(aPrivateUser, sizeof(aPrivateUser), "%s/user", pPrivateRoot);
	if(!AndroidUserDirHasConfig(aPrivateUser))
		return;
	fs_makedir("user");
	SCopyTree Tree;
	str_copy(Tree.m_aFrom, aPrivateUser);
	str_copy(Tree.m_aTo, "user");
	fs_listdir(Tree.m_aFrom, CopyTreeEntry, 0, &Tree);
	log_info("android", "Copied saved data from '%s' into the BestClient folder", Tree.m_aFrom);
}
// bestclient-light

const char *InitAndroid()
{
	// bestclient-light
	char aPrivateRoot[IO_MAX_PATH_LENGTH];
	const char *pExternal = SDL_AndroidGetExternalStoragePath();
	str_copy(aPrivateRoot, pExternal != nullptr ? pExternal : "");
	char aPersistent[IO_MAX_PATH_LENGTH];
	const char *pPath = nullptr;
	if(AndroidPersistentRoot(aPersistent, sizeof(aPersistent)))
		pPath = aPersistent;
	else
		pPath = pExternal;
	// bestclient-light
	if(pPath == nullptr)
	{
		return "The external storage is not available.";
	}
	if(fs_chdir(pPath) != 0)
	{
		return "Failed to change current directory to external storage.";
	}
	log_info("android", "Changed current directory to '%s'", pPath);
	// bestclient-light
	if(pPath == aPersistent && str_comp(aPersistent, aPrivateRoot) != 0)
		MigratePrivateUserDir(aPrivateRoot);
	// bestclient-light

	if(fs_makedir("data") != 0 || fs_makedir("user") != 0)
	{
		return "Failed to create 'data' and 'user' directories in external storage.";
	}

	if(EqualIntegrityFiles(INTEGRITY_INDEX, INTEGRITY_INDEX_SAVE))
	{
		return nullptr;
	}

	if(!UnpackAsset(INTEGRITY_INDEX))
	{
		return "Failed to unpack the integrity index file. Consider reinstalling the app.";
	}

	std::vector<CIntegrityFileLine> vIntegrityLines = ReadIntegrityFile(INTEGRITY_INDEX);
	if(vIntegrityLines.empty())
	{
		return "Failed to load the integrity index file. Consider reinstalling the app.";
	}

	std::vector<CIntegrityFileLine> vIntegritySaveLines = ReadIntegrityFile(INTEGRITY_INDEX_SAVE);

	// The remaining lines of each integrity file list all assets and their hashes
	for(size_t i = 1; i < vIntegrityLines.size(); ++i)
	{
		const CIntegrityFileLine &IntegrityLine = vIntegrityLines[i];

		// Check if the asset is unchanged from the last unpacking
		const auto IntegritySaveLine = std::find_if(vIntegritySaveLines.begin(), vIntegritySaveLines.end(), [&](const CIntegrityFileLine &Line) {
			return str_comp(Line.m_aFilename, IntegrityLine.m_aFilename) == 0;
		});
		if(IntegritySaveLine != vIntegritySaveLines.end() && IntegritySaveLine->m_Sha256 == IntegrityLine.m_Sha256)
		{
			continue;
		}

		if(fs_makedir_rec_for(IntegrityLine.m_aFilename) != 0 || !UnpackAsset(IntegrityLine.m_aFilename))
		{
			return "Failed to unpack game assets, consider reinstalling the app.";
		}
	}

	for(size_t i = 1; i < vIntegritySaveLines.size(); ++i)
	{
		const CIntegrityFileLine &IntegritySaveLine = vIntegritySaveLines[i];

		// Check if the asset was deleted since the last unpacking
		const bool LineFound = std::any_of(vIntegrityLines.begin(), vIntegrityLines.end(), [&](const CIntegrityFileLine &Line) {
			return str_comp(Line.m_aFilename, IntegritySaveLine.m_aFilename) == 0;
		});
		if(LineFound)
		{
			continue;
		}

		if(fs_remove(IntegritySaveLine.m_aFilename) != 0)
		{
			log_warn("android", "Failed to delete unused asset '%s'", IntegritySaveLine.m_aFilename);
		}
	}

	// The integrity file will be unpacked every time when launching,
	// so we can simply rename it to update the saved integrity file.
	if(fs_remove(INTEGRITY_INDEX_SAVE) != 0 || fs_rename(INTEGRITY_INDEX, INTEGRITY_INDEX_SAVE) != 0)
	{
		return "Failed to update the saved integrity index file.";
	}

	return nullptr;
}

// See ClientActivity.java
constexpr uint32_t COMMAND_USER = 0x8000;
constexpr uint32_t COMMAND_RESTART_APP = COMMAND_USER + 1;

void RestartAndroidApp()
{
	SDL_AndroidSendMessage(COMMAND_RESTART_APP, 0);
}

bool StartAndroidServer(const char **ppArguments, size_t NumArguments)
{
	// We need the notification-permission to show a notification for the foreground service.
	// We use SDL for this instead of doing it on the Java side because this function blocks
	// until the user made a choice, which is easier to handle. Only Android 13 (API 33) and
	// newer support requesting this permission at runtime.
	if(SDL_GetAndroidSDKVersion() >= 33 && !SDL_AndroidRequestPermission("android.permission.POST_NOTIFICATIONS"))
	{
		return false;
	}

	JNIEnv *pEnv = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
	jobject Activity = (jobject)SDL_AndroidGetActivity();
	jclass ActivityClass = pEnv->GetObjectClass(Activity);

	jclass StringClass = pEnv->FindClass("java/lang/String");
	jobjectArray ArgumentsArray = pEnv->NewObjectArray(NumArguments, StringClass, nullptr);
	pEnv->DeleteLocalRef(StringClass);
	for(size_t ArgumentIndex = 0; ArgumentIndex < NumArguments; ArgumentIndex++)
	{
		jstring ArgumentString = pEnv->NewStringUTF(ppArguments[ArgumentIndex]);
		pEnv->SetObjectArrayElement(ArgumentsArray, ArgumentIndex, ArgumentString);
		pEnv->DeleteLocalRef(ArgumentString);
	}

	jmethodID MethodId = pEnv->GetMethodID(ActivityClass, "startServer", "([Ljava/lang/String;)V");
	pEnv->CallVoidMethod(Activity, MethodId, ArgumentsArray);

	pEnv->DeleteLocalRef(ArgumentsArray);
	pEnv->DeleteLocalRef(Activity);
	pEnv->DeleteLocalRef(ActivityClass);

	return true;
}

void ExecuteAndroidServerCommand(const char *pCommand)
{
	JNIEnv *pEnv = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
	jobject Activity = (jobject)SDL_AndroidGetActivity();
	jclass ActivityClass = pEnv->GetObjectClass(Activity);

	jstring Command = pEnv->NewStringUTF(pCommand);

	jmethodID MethodId = pEnv->GetMethodID(ActivityClass, "executeCommand", "(Ljava/lang/String;)V");
	pEnv->CallVoidMethod(Activity, MethodId, Command);

	pEnv->DeleteLocalRef(Command);
	pEnv->DeleteLocalRef(Activity);
	pEnv->DeleteLocalRef(ActivityClass);
}

bool IsAndroidServerRunning()
{
	JNIEnv *pEnv = static_cast<JNIEnv *>(SDL_AndroidGetJNIEnv());
	jobject Activity = (jobject)SDL_AndroidGetActivity();
	jclass ActivityClass = pEnv->GetObjectClass(Activity);

	jmethodID MethodId = pEnv->GetMethodID(ActivityClass, "isServerRunning", "()Z");
	const bool Result = pEnv->CallBooleanMethod(Activity, MethodId);

	pEnv->DeleteLocalRef(Activity);
	pEnv->DeleteLocalRef(ActivityClass);

	return Result;
}
