package org.ddnet.client;

import android.app.NativeActivity;
import android.content.*;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.*;
import android.provider.DocumentsContract;
import android.provider.Settings;
import android.widget.Toast;

import androidx.core.content.ContextCompat;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.util.concurrent.atomic.AtomicBoolean;

public class ClientActivity extends SDLActivity {

	private static final int COMMAND_RESTART_APP = SDLActivity.COMMAND_USER + 1;
	private static final int REQUEST_LEGACY_STORAGE = 4101;

	private String[] launchArguments = new String[0];
	private final AtomicBoolean m_WaitingForStoragePermission = new AtomicBoolean(false);

	private final Object serverServiceMonitor = new Object();
	private Messenger serverServiceMessenger = null;
	private final ServiceConnection serverServiceConnection = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			synchronized(serverServiceMonitor) {
				serverServiceMessenger = new Messenger(service);
			}
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			synchronized(serverServiceMonitor) {
				serverServiceMessenger = null;
			}
		}
	};

	@Override
	protected String[] getLibraries() {
		return new String[] {
			"DDNet",
		};
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

		Intent intent = getIntent();
		if(intent != null) {
			String gfxBackend = intent.getStringExtra("gfx-backend");
			if(gfxBackend != null) {
				if(gfxBackend.equals("Vulkan")) {
					launchArguments = new String[] {"gfx_backend Vulkan"};
				} else if(gfxBackend.equals("GLES")) {
					launchArguments = new String[] {"gfx_backend GLES"};
				}
			}
		}

		super.onCreate(savedInstanceState);
	}

	@Override
	protected void onResume() {
		super.onResume();
		if(m_WaitingForStoragePermission.get() && hasAllFilesAccess()) {
			m_WaitingForStoragePermission.set(false);
			restartApp();
		}
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults);
		if(requestCode == REQUEST_LEGACY_STORAGE && hasAllFilesAccess()) {
			m_WaitingForStoragePermission.set(false);
			restartApp();
		}
	}

	private boolean hasAllFilesAccess() {
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
			return Environment.isExternalStorageManager();
		return checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
	}

	private void openStorageSettings() {
		try {
			if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
				try {
					Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
					intent.setData(Uri.parse("package:" + getPackageName()));
					startActivity(intent);
					return;
				} catch(ActivityNotFoundException ignored) {
					Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
					startActivity(intent);
					return;
				}
			}
			requestPermissions(new String[] {android.Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_LEGACY_STORAGE);
		} catch(RuntimeException ignored) {
		}
	}

	// Called from native code before the save directory is chosen.
	public String persistentStoragePath() {
		if(!hasAllFilesAccess()) {
			if(m_WaitingForStoragePermission.compareAndSet(false, true))
				runOnUiThread(this::openStorageSettings);
			return null;
		}
		File Dir = new File(Environment.getExternalStorageDirectory(), "BestClient");
		if(!Dir.exists() && !Dir.mkdirs())
			return null;
		return Dir.getAbsolutePath();
	}

	// Called from native code. Opens the real folder in the system files app.
	public boolean openFolder(String path) {
		if(path == null || path.isEmpty())
			return false;
		runOnUiThread(() -> openFolderOnUi(path));
		return true;
	}

	private boolean tryViewFolder(Uri folderUri) {
		String[] types = new String[] {DocumentsContract.Document.MIME_TYPE_DIR, "resource/folder"};
		for(String type : types) {
			Intent viewIntent = new Intent(Intent.ACTION_VIEW);
			viewIntent.setDataAndType(folderUri, type);
			try {
				startActivity(viewIntent);
				return true;
			} catch(ActivityNotFoundException ignored) {
			} catch(RuntimeException ignored) {
			}
		}
		return false;
	}

	private void openFolderOnUi(String path) {
		File dir = new File(path);
		if(!dir.exists())
			dir.mkdirs();
		File root = Environment.getExternalStorageDirectory();
		String rootPath = root.getAbsolutePath();
		while(rootPath.endsWith("/"))
			rootPath = rootPath.substring(0, rootPath.length() - 1);
		boolean opened = false;
		if(path.equals(rootPath) || path.startsWith(rootPath + "/")) {
			String relative = path.substring(rootPath.length());
			while(relative.startsWith("/"))
				relative = relative.substring(1);
			while(relative.endsWith("/"))
				relative = relative.substring(0, relative.length() - 1);
			String docId = "primary:" + relative;
			opened = tryViewFolder(DocumentsContract.buildDocumentUri("com.android.externalstorage.documents", docId));
			if(!opened)
				opened = tryViewFolder(DocumentsContract.buildTreeDocumentUri("com.android.externalstorage.documents", docId));
		}
		if(!opened)
			Toast.makeText(this, path, Toast.LENGTH_LONG).show();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		synchronized(serverServiceMonitor) {
			if(serverServiceMessenger != null) {
				unbindService(serverServiceConnection);
			}
		}
	}

	@Override
	protected String[] getArguments() {
		return launchArguments;
	}

	@Override
	protected boolean onUnhandledMessage(int command, Object param) {
		switch(command) {
		case COMMAND_RESTART_APP:
			restartApp();
			return true;
		}
		return false;
	}

	private void restartApp() {
		Intent restartIntent =
			Intent.makeRestartActivityTask(
				getPackageManager().getLaunchIntentForPackage(
					getPackageName()
				).getComponent()
			);
		restartIntent.setPackage(getPackageName());
		startActivity(restartIntent);
	}

	// Called from native code, see android_main.cpp
	public void startServer(String[] arguments) {
		synchronized(serverServiceMonitor) {
			if(serverServiceMessenger != null) {
				return;
			}
			Intent startIntent = ServerService.createStartIntent(this, arguments);
			ContextCompat.startForegroundService(this, startIntent);
			bindService(startIntent, serverServiceConnection, 0);
		}
	}

	// Called from native code, see android_main.cpp
	public void executeCommand(String command) {
		synchronized(serverServiceMonitor) {
			if(serverServiceMessenger == null) {
				return;
			}
			try {
				serverServiceMessenger.send(ServerService.createExecuteCommandMessage(command));
			} catch (RemoteException e) {
				// Connection broken
				unbindService(serverServiceConnection);
			}
		}
	}

	// Called from native code, see android_main.cpp
	public boolean isServerRunning() {
		synchronized(serverServiceMonitor) {
			return serverServiceMessenger != null;
		}
	}
}
