import { Component, OnInit, Input} from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Subscription } from 'rxjs';
import { BackgroundProcessManager } from '@app/common/BackgroundProcessManager';
import { Utility } from '@app/common/Utility';

/*
This widget shows the upload progress of a file upload going on in the background.
Widget has progress bar which on click redirects to the upload page, stop button to cancel upload.
BackgroundProcessManager does the actual upload handling.
The controller service subscribes to BACKGROUND_FILEUPLOAD_PROGRESS and updates the progress bar using the payload.
On clicking cancel button we get the instance from BackgroundProcessManager and call abortFileUpload(), onCancelFileUpload(), unRegisterVeniceImageFileUpload().

BACKGROUND_FILEUPLOAD_SUCCESS, BACKGROUND_FILEUPLOAD_FAILURE, BACKGROUND_FILEUPLOAD_CANCEL make the widget dissappear.
*/

@Component({
  selector: 'app-bgfileupload',
  templateUrl: './bgfileupload.component.html',
  styleUrls: ['./bgfileupload.component.scss']
})
export class BgfileuploadComponent implements OnInit {

  subscriptions: Subscription[] = [];
  fileUploadProgress: number;
  showWidget: boolean = false;
  componentClosed: boolean = false;
  filename: string;
  @Input() showValue: boolean;
  @Input() styleClass: any = {};

  constructor(protected controllerService: ControllerService) {
  }

  ngOnInit() {
    this.fileUploadProgress = 0;
    this.subcribeToBackgroundFileUploadEvents();
  }

  subcribeToBackgroundFileUploadEvents() {
    const subSuccess = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_SUCCESS,  (payload) => {
      this.onBackgroudUploadSuccess(payload);
    });
    this.subscriptions.push(subSuccess);
    const subFailure = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_FAILURE,  (payload) => {
      this.onBackgroudUploadFailure(payload);
    });
    this.subscriptions.push(subFailure);
    const subProgress = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_PROGRESS,  (payload) => {
      this.onBackgroudUploadProgress(payload);
    });
    this.subscriptions.push(subProgress);
    const subCancel = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_CANCEL,  (payload) => {
      this.onBackgroudUploadCancel(payload);
    });
    this.subscriptions.push(subCancel);
    this.subscriptions[Eventtypes.COMPONENT_INIT] = this.controllerService.subscribe(Eventtypes.COMPONENT_INIT, (payload) => {
      this.onComponentStateChangeInit(payload);
    });
    this.subscriptions[Eventtypes.COMPONENT_DESTROY] = this.controllerService.subscribe(Eventtypes.COMPONENT_DESTROY, (payload) => {
      this.onComponentStateChangeDestroy(payload);
    });
  }

  private onComponentStateChangeInit(payload: any) {
    if (payload.component === 'ImageuploadComponent') {
      this.showWidget = false;
      this.componentClosed = false;
    }
  }

  private onComponentStateChangeDestroy(payload: any) {
    if (payload.component === 'ImageuploadComponent') {
      this.componentClosed = true;
    }
  }

  private onBackgroudUploadCancel(payload: any) {
    this.showWidget = false;
    this.resetProgressStatus();
  }

  private onBackgroudUploadSuccess(payload: any) {
    this.showWidget = false;
    this.resetProgressStatus();
  }

  private onBackgroudUploadFailure(payload: any) {
    this.showWidget = false;
    this.resetProgressStatus();
  }

  private onBackgroudUploadProgress(payload: any) {
    // Currently supports single file upload
    this.filename = payload.files[0];
    this.showWidget = true && this.componentClosed;
    const progress = (payload.progress && payload.progress >= this.fileUploadProgress ) ? payload.progress : 0;
    this.fileUploadProgress = progress;
  }

  private resetProgressStatus() {
    this.fileUploadProgress = 0;
  }

  onClickCancel() {
    this.controllerService.invokeConfirm({
      header: 'Are you sure you want to cancel file upload?',
      message: 'Stop uploading ' + this.filename + ' ?',
      acceptLabel: 'Confirm',
      accept: () => {
        this.resetProgressStatus();
        BackgroundProcessManager.getInstance().abortFileUpload();
        BackgroundProcessManager.getInstance().onCancelFileUpload();
        BackgroundProcessManager.getInstance().unRegisterVeniceImageFileUpload();

    }});

  }

  redirectOnClick() {
    window.location.href = Utility.getBaseUIUrl() + '/admin/upgrade/imageupload';

  }
}
