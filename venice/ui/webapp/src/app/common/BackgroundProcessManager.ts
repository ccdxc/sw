
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '../services/controller.service';
import { Utility } from './Utility';

export interface BackgroundFileUpload {
    type: BackgroundProcessType;
    xhr: XMLHttpRequest;
    progress: number;
    files?: any[]; // should be 'File' object.
    error?: any;
}

export enum BackgroundProcessType {
    UPLOAD_VENICE_IMAGE_FILE
}

/**
 * BackgroundProcessManager is responsible for managing background process.
 * It is a singleton class.  To use it, BackgroundProcessManager.getInstance().xxx()
 *
 * 2019-05-29, we implemented "upload Venice image file in background" feature. Imageupload.comonent.ts uses it.  This is the only use case for now.
 *
 * We will refactor and expand this class as there will be more use cases .
 */


export class BackgroundProcessManager {

    static instance: BackgroundProcessManager;

    myBackgroundVeniceImageFileUpload: BackgroundFileUpload;
    myControllerService: ControllerService;
    uploadCancelled: boolean = false;

    private constructor() { }

    public static getInstance(): BackgroundProcessManager {
        if (!this.instance) {
            this.instance = new BackgroundProcessManager();
        }
        return this.instance;
    }

    getControllerService(): ControllerService {
        if (!this.myControllerService) {
            this.myControllerService = Utility.getInstance().getControllerService();
        }
        return this.myControllerService;
    }

    getBackgroundVeniceImageFileUpload(): BackgroundFileUpload {
        return this.myBackgroundVeniceImageFileUpload;
    }

    registerVeniceImageFileUpload(xhr: XMLHttpRequest, files: any) {
        this.uploadCancelled = false;
        this.myBackgroundVeniceImageFileUpload = {
            type: BackgroundProcessType.UPLOAD_VENICE_IMAGE_FILE,
            xhr: xhr,
            progress: 0,
            error: null,
            files: files,
        };
        xhr.upload.addEventListener('progress', (e: ProgressEvent) => {
            if (e.lengthComputable && this.myBackgroundVeniceImageFileUpload ) {
                this.myBackgroundVeniceImageFileUpload.progress = Math.round((e.loaded * 100) / e.total);
                // for debug console.log('BackgroundProcessManager.registerFileUpload() file upload progress ' + this.myBackgroundVeinceImageFileUpload.progress);
                this.getControllerService().publish(Eventtypes.BACKGROUND_FILEUPLOAD_PROGRESS, { xhr: xhr, files: files, status: 'progress', progress: this.myBackgroundVeniceImageFileUpload.progress });
            }
        }, false);

        xhr.onreadystatechange = () => {
            if (xhr.readyState === 4) {
                this.myBackgroundVeniceImageFileUpload.progress = 0;
                const fileNames = this.getFilesNames(this.myBackgroundVeniceImageFileUpload.files);
                if (xhr.status >= 200 && xhr.status < 300) {
                    this.getControllerService().invokeInfoToaster('Upload', 'File is uploaded. ' + this.getFilesNames(this.myBackgroundVeniceImageFileUpload.files));
                    this.getControllerService().publish(Eventtypes.BACKGROUND_FILEUPLOAD_SUCCESS, { xhr: xhr, files: files, status: 'success' });
                } else if (!this.uploadCancelled) {
                    this.getControllerService().invokeErrorToaster('Upload', 'Failed to upload file. ' + this.getFilesNames(this.myBackgroundVeniceImageFileUpload.files));
                    this.getControllerService().publish(Eventtypes.BACKGROUND_FILEUPLOAD_FAILURE, { xhr: xhr, files: files, status: 'failure' });
                }
                this.unRegisterVeniceImageFileUpload() ;  // VS-498 remove reference
            }
        };
    }

    getFilesNames(files: any[]): string[] {
        const filenames: string[] = [];
        for (const file of files) {
           const fname = (file.name) ? file.name : file;
          filenames.push(fname);
        }
        return filenames;
      }

    abortFileUpload() {
        this.uploadCancelled = true;
        if (this.myBackgroundVeniceImageFileUpload.xhr) {
            this.myBackgroundVeniceImageFileUpload.xhr.abort();
            this.myBackgroundVeniceImageFileUpload.xhr = null;
        }
    }

    onCancelFileUpload() {
        // File information is not included in the payload right now, can be added to the toaster message in the future.
        this.getControllerService().invokeInfoToaster('Upload', 'Upload operation was cancelled');
        this.getControllerService().publish(Eventtypes.BACKGROUND_FILEUPLOAD_CANCEL, { status: 'cancel' });
    }

    unRegisterVeniceImageFileUpload() {
        this.myBackgroundVeniceImageFileUpload = null;
    }
}
