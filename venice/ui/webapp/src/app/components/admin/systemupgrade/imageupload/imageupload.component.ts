import { Component, OnInit, ViewEncapsulation, ChangeDetectorRef, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { Observable } from 'rxjs';

import { ControllerService } from '@app/services/controller.service';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { IApiStatus, IObjstoreObject, ObjstoreObject, IObjstoreObjectList, ObjstoreObjectList } from '@sdk/v1/models/generated/objstore';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Utility } from '@app/common/Utility';
import { BackgroundProcessManager } from '@app/common/BackgroundProcessManager';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { AUTH_KEY } from '@app/core';
import { RolloutUtil } from '@app/components/admin/systemupgrade/rollouts/RolloutUtil';
import { RolloutImageLabel } from '@app/components/admin/systemupgrade/rollouts/';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { RolloutService } from '@app/services/generated/rollout.service';
import { RolloutRollout } from '@sdk/v1/models/generated/rollout';
import { UIConfigsService } from '@app/services/uiconfigs.service';

/**
 * This component let user upload Venice rollout images and manage existing images.
 *
 * Uploading image file takes time.  Thus, we enable file uploading running in background.  User can visit other page when file upload is initiated.
 * ImageuploadComponent.ngDestroy() invokes BackgroundProcessManager.getInstance().registerVeniceImageFileUpload() which will manage uploading process and publish status event.
 * When user comes back to ImageuploadComponent page, ImageuploadComponent.subcribeToBackgroundFileUploadEvents() will listens to events published by BackgroundProcessManager.registerVeniceImageFileUpload()
 * ImageuploadComponent.html will handle the UI display accordingly.
 *
 * ImageuploadComponent.processRolloutImages() is a key API that handles how to display images.  (how UI works with backend)
 *
 * Note:
 *   imageupload.component.html use <p-fileUpload> widget which not using Angular http-clinet. We have to use p-fileUpload.onBeforeSend() hook to add security-token.
 *   We change UI according to file upload progress. User can cancel upload. If user move away from current page. We will cancel file upload.
 *
 *
 *   As well, this component lists rollout images and let user delete images.
 *
 */

@Component({
  selector: 'app-imageupload',
  templateUrl: './imageupload.component.html',
  styleUrls: ['./imageupload.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})

export class ImageuploadComponent extends TablevieweditAbstract<IObjstoreObject, ObjstoreObject> implements OnInit, OnDestroy {

  dataObjects: ReadonlyArray<ObjstoreObject> = [];
  isTabComponent: boolean = false;
  exportMap: CustomExportMap = {};
  disableTableWhenRowExpanded: boolean = true;
  loading: boolean = false;

  uploadedFiles: any[] = [];
  uploadingFiles: string[] = [];
  fileUploadProgress: number = 0;

  _uploadCancelled: boolean = false;
  showUploadButton: boolean = true;
  uploadInForeground: boolean = false;
  _xhr: XMLHttpRequest = null;

  bodyIcon: Icon = {
    margin: {
      top: '8px',
      left: '10px',
    },
    matIcon: 'cloud_upload'
  };

  startingSortField: string = 'meta.mod-time';
  startingSortOrder: number = -1;

  cols: TableCol[] = [
    { field: 'meta.labels.Version', header: 'Version', class: 'imageupload-column-common imageupload-column-version', sortable: true, width: 40 },
    { field: 'meta.labels', header: 'Details', class: 'imageupload-column-common imageupload-column-labels', sortable: false, width: 40 },
    { field: 'meta.mod-time', header: 'Time', class: 'imageupload-column-common imageupload-column-date', sortable: true, width: 20 },
  ];

  exportFilename: string = 'Venice-images';

  rolloutsEventUtility: HttpEventUtility<RolloutRollout>;
  rollouts: ReadonlyArray<RolloutRollout>;
  activeImageVersions: string[] = [];

  constructor(protected controllerService: ControllerService,
    private objstoreService: ObjstoreService,
    protected cdr: ChangeDetectorRef,
    private rolloutService: RolloutService,
    protected uiconfigsService: UIConfigsService
  ) {
    super(controllerService, cdr, uiconfigsService);
  }

  getClassName(): string {
    return this.constructor.name;
  }
  setDefaultToolbar(): void {
    const breadcrumb = [{ label: 'System Upgrade', url: Utility.getBaseUIUrl() + 'admin/upgrade' }, { label: 'Images', url: '' }];
    const buttons = [];

    buttons.push(
      {
        cssClass: 'global-button-primary imageupload-toolbar-button imageupload-toolbar-button-refresh',
        text: 'REFRESH',
        callback: () => { this.refreshImages(); },
      }
    );

    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: breadcrumb
    });
  }

  postNgInit() {
    this.controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'ImageuploadComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.getRolloutImages();
    this.subcribeToBackgroundFileUploadEvents();
  }

  subcribeToBackgroundFileUploadEvents() {
    const subSuccess = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_SUCCESS, (payload) => {
      this.onBackgroudUploadSuccess(payload);
    });
    this.subscriptions.push(subSuccess);
    const subFailure = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_FAILURE, (payload) => {
      this.onBackgroudUploadFailure(payload);
    });
    this.subscriptions.push(subFailure);
    const subProgress = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_PROGRESS, (payload) => {
      this.onBackgroudUploadProgress(payload);
    });
    this.subscriptions.push(subProgress);
    const subCancel = this.controllerService.subscribe(Eventtypes.BACKGROUND_FILEUPLOAD_CANCEL, (payload) => {
      this.onBackgroudUploadCancel(payload);
    });
    this.subscriptions.push(subCancel);
  }
  private onBackgroudUploadCancel(payload: any) {
    this.refreshImages();
    this.resetProgressStatus();
  }

  private onBackgroudUploadSuccess(payload: any) {
    this.refreshImages();
    this.resetProgressStatus();
  }

  private onBackgroudUploadFailure(payload: any) {
    this.resetProgressStatus();
  }

  private onBackgroudUploadProgress(payload: any) {
    const progress = (payload.progress) ? payload.progress : 0;
    this.fileUploadProgress = (progress > this.fileUploadProgress) ? progress : this.fileUploadProgress;
  }


  /** Override parent API as watchRolloutImages() is not working. */
  postDeleteRecord() {
    this.refreshImages();
  }

  refreshImages() {
    this.getRolloutImages();
  }

  /**
   * 2019-05-15.
   * Once a bundle.tar is uploaded, back-end will un-tar it. Un-taring will have a matadata.json
   * We want take labels out fo metadata.json and add to bundle.tar
   * The image list table will only display bundle.tar
   *
   * 2019-06-05
   * User can upload two bundle.tar files. 'bundle.tar' is fixed name. Logically, let us say bundle.tar-1, bundle.tar-2.
   * bundle.tar-1 internally is version-1.0.1, bundle.tar-2 internally is version-2.0.a
   * ListObject REST API will return two metadata.json.  We process the the json such that the image list table show two rows
   * Also, we arrange the delete url as
   *  https://10.7.100.39:10001/objstore/v1/tenant/default/images/objects/0version-1.0.1  // version-string is at the end of url
   *
   */

  getRolloutImages() {
    const sub = this.objstoreService.ListObject(Utility.ROLLOUT_IMGAGE_NAMESPACE).subscribe(
      (response) => {
        this.processRolloutImages(response);
      },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Failed to fetch rollout images.', error);
      }
    );
    this.subscriptions.push(sub);
  }

  /**
   *
   * @param response
   */
  processRolloutImages(response) {
    /**
     * 2019-05-xx implementation logic
     * once bundle.tar is uploaded, backend will un-tar it to multiple files. objstoreService.listXXX()/Watch() will bring back multiple files, including bundle.tar and metadat.json
     * We get labels information from metadata.json,  attach metadata.labels to bundle.tar. We display bundle+version in table only. If user delete bundle.tar, backend will handle deleting the un-tar files.
     */
    let metaImage: ObjstoreObject = null;
    let rolloutImages: IObjstoreObjectList = null;
    rolloutImages = (response) ? response.body as IObjstoreObjectList : rolloutImages;
    if (rolloutImages && rolloutImages.items) {
      const entries = [];
      const allentries = [];
      rolloutImages.items.forEach((image: ObjstoreObject) => {
        if (image.meta.name.endsWith(RolloutUtil.ROLLOUT_METADATA_JSON)) {
          metaImage = image;
          entries.push(metaImage);
        }
        allentries.push(image);
      });
      // in case user uploads a dummy tar file (not bundle.tar), we will show it. So that user can delete it. // 2019-05-17 // TODO: clean it up once back-end is ok.
      this.dataObjects = (entries.length > 0) ? entries : allentries;
    } else {
      this.dataObjects = []; // it possible that server has no rolloutImage.
    }
    this.getRollouts();
  }

  onBeforeSend($event) {
    this.uploadInForeground = true;
    const xhr = $event.xhr;
    this._xhr = xhr;
    const headerName = AUTH_KEY;
    const token = Utility.getInstance().getXSRFtoken();
    xhr.setRequestHeader(headerName, token);
    this._uploadCancelled = false;
    this.showUploadButton = false;
    this.controllerService.invokeInfoToaster('Upload', 'File upload started.  Please do not leave Venice site');
  }
  /**
   * This API serves html template
   */
  onUpload(event) {
    const files = event.files;
    const filenames = this.getFilesNames(files);
    this.controllerService.invokeInfoToaster('Upload succeeded', 'Uploaded files: ' + filenames.join(',') + '. System will validate uploaded files.');
    this.refreshImages();
    this.resetProgressStatus();
  }

  /**
  * This API serves html template
  *
  * If user cancels upload operation, it will trigger onError() call.  We don't want to popup error toaster.
  * We will pop up error toaster if server response error. (such as server disk space issue, file handling errors, etc)
  */
  onError(event) {
    if (this._uploadCancelled) {
      return;
    }
    const files = event.files;
    const filenames = this.getFilesNames(files);
    // debug event object to see event.xhr.  VS-448
    const msg = (event && event.xhr && event.xhr.status && event.xhr.response) ? filenames.join(',') + '\n' + event.xhr.response : filenames.join(',');
    this.controllerService.invokeErrorToaster('Upload failed', msg);
    this.resetProgressStatus();
  }

  /**
  * This API serves html template
  */
  onCancelUpload() {
    if (this.isFileUploadInProgress()) {
      this.cancelUpload();  // cancel upload only when upload started.
    }
  }

  /**
  * This API serves html template
  */
  onFileRemove($event) {
    if ($event && $event.file) {
      if (this.isFileUploadInProgress()) {
        this.cancelUpload();
      }
    }
  }

  cancelUpload() {
    this._uploadCancelled = true;
    if (this._xhr) {
      this._xhr.abort();
    }
    this.resetProgressStatus();
    this.controllerService.invokeInfoToaster('Upload', 'Upload operation was cancelled');
    this.controllerService.publish(Eventtypes.BACKGROUND_FILEUPLOAD_CANCEL, { status: 'cancel' });
  }

  onFileSelect($event) {
    // Just uploading one file.
    if ($event.files.length > 0) {
      this.uploadingFiles.push($event.files[0]['name']);
    }
    this.fileUploadProgress = 0;
  }

  private resetProgressStatus() {
    if (this.uploadedFiles) {
      this.uploadedFiles.length = 0;
    }
    this.fileUploadProgress = 0;
    this.showUploadButton = true;
    this._xhr = null;
    BackgroundProcessManager.getInstance().unRegisterVeniceImageFileUpload();
  }

  getFilesNames(files: any[]): string[] {
    const filenames: string[] = [];
    for (const file of files) {
      this.uploadedFiles.push(file);
      filenames.push(file.name);
    }
    return filenames;
  }

  /**
   * This api servers html template.
   * @param object
   * object JSON is
   * "{"kind":"Object","meta":{"name":"Bundle/0.14.0-E-5_img/metadata.json","generation-id":"","labels":{"Releasedata":"","Version":"0.14.0-E-5"},"creation-time":"1970-01-01T00:00:00Z","mod-time":"2019-07-23T19:29:46Z"},"spec":{"content-type":"application/octet-stream"},"status":{"size":932,"digest":"262ee56098e9ab1a0498ad129feff5e1-1"}}"
   */
  showDeleteButton(object: IObjstoreObject): boolean {
    // VS-484. We don't want to delete an image that is lined to a pending rollout;
    const version =  this.getBundlerTarVersion(object); //  get Version  => "0.14.0-E-5"
    if (this.activeImageVersions.includes(version)) {
      return false;
    } else {
      return true;
    }
  }

  generateDeleteSuccessMsg(object: IObjstoreObject): string {
    return 'Deleted Image ' + object.meta.name;
  }

  deleteRecord(object: IObjstoreObject): Observable<{ body: IObjstoreObject | IApiStatus | Error, statusCode: number }> {
    return this.objstoreService.DeleteObject(Utility.ROLLOUT_IMGAGE_NAMESPACE, this.getBundlerTarVersion(object));
  }

  getBundlerTarVersion(object: IObjstoreObject): string {
    return (object.meta.labels && object.meta.labels['Version']) ? object.meta.labels['Version'] : object.meta.name;
  }

  generateDeleteConfirmMsg(object: IObjstoreObject) {
    return 'Are you sure you want to delete image: ' + this.getBundlerTarVersion(object);
  }

  displayColumn(record, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(record, fields);
    const column = col.field;
    switch (column) {
      case 'meta.name':
        return 'bundle ' + this.getBundlerTarVersion(record);  // VS-499
      case 'meta.labels':
        return (value) ? Utility.displaySimpleObject(value) : '';
      case 'meta.labels.Version':
        return (value) ? value : '';
      default:
        return JSON.stringify(value, null, 2);
    }
  }

  onUploadProgress($event) {
    if (!this._uploadCancelled) {
      const progress = $event.progress;
      this.fileUploadProgress = (progress > this.fileUploadProgress) ? progress : this.fileUploadProgress;
    }
  }

  ngOnDestroy() {
    this.controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'ImageuploadComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    if (this.isFileUploadInProgress() && this.uploadInForeground) {
      //  When user leaves upload image page, we don't cancel upload (//this.cancelUpload()), instead we put upload process in background.
      BackgroundProcessManager.getInstance().registerVeniceImageFileUpload(this._xhr, this.uploadingFiles);
      this.controllerService.invokeInfoToaster('Upload', 'File upload will be running in background.  Please do not leave Venice site');
    }
    super.ngOnDestroy();
  }

  private isFileUploadInProgress(): boolean {
    return (this._xhr !== null) || (this.fileUploadProgress > 0);
  }

  isThereBackgroudFileUpload(): boolean {
    const backgroundFileUpload = BackgroundProcessManager.getInstance().getBackgroundVeniceImageFileUpload();
    return !!(backgroundFileUpload);
  }

  getRollouts() {
    this.rolloutsEventUtility = new HttpEventUtility<RolloutRollout>(RolloutRollout, true);
    this.rollouts = this.rolloutsEventUtility.array as ReadonlyArray<RolloutRollout>;
    const subscription = this.rolloutService.WatchRollout().subscribe(
      response => {
        this.rolloutsEventUtility.processEvents(response);
        this.setDefaultToolbar();
        this.splitRollouts();
      },
      this.controllerService.webSocketErrorHandler('Failed to get Rollouts info')
    );
    this.subscriptions.push(subscription);
  }

  splitRollouts() {
    this.activeImageVersions.length = 0;
    for  (let i = 0; i < this.rollouts.length; i++) {
      if ( !RolloutUtil.isRolloutInactive(this.rollouts[i]) ) {
        this.activeImageVersions.push(this.rollouts[i].spec.version); //  version is like  "0.14.0-E-5"
      }
    }
  }
}
