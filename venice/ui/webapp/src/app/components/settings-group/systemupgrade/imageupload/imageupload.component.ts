import { Component, OnInit, ViewEncapsulation, ChangeDetectorRef, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { Observable } from 'rxjs';

import { ControllerService } from '@app/services/controller.service';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { IApiStatus, IObjstoreObject, ObjstoreObject, IObjstoreObjectList, ObjstoreObjectList } from '@sdk/v1/models/generated/objstore';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Utility } from '@app/common/Utility';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { AUTH_KEY } from '@app/core';
import { RolloutUtil } from '@app/components/settings-group/systemupgrade/rollouts/RolloutUtil';
import { RolloutImageLabel } from '@app/components/settings-group/systemupgrade/rollouts/';
import { TableCol } from '@app/components/shared/tableviewedit';

/**
 * This component let user upload Venice rollout images and manage existing images.
 * Note:
 *   imageupload.component.html use <p-fileUpload> widget which not using Angular http-clinet. We have to use p-fileUpload.onBeforeSend() hook to add security-token.
 *   We change UI according to file upload progress. User can cancel upload. If user move away from current page. We will cancel file upload.
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

export class ImageuploadComponent extends TablevieweditAbstract<IObjstoreObject, ObjstoreObject> implements OnInit , OnDestroy {

  dataObjects: ReadonlyArray<ObjstoreObject> = [];
  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = true;
  rolloutimagesEventUtility: HttpEventUtility<ObjstoreObject>;
  loading: boolean = false;

  uploadedFiles: any[] = [];
  fileUploadProgress: number = 0;

  _uploadCancelled: boolean = false;
  showUploadButton: boolean = true;

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
    { field: 'meta.name', header: 'Name', class: 'imageupload-column-common imageupload-column-who', sortable: true, width: 10 },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'imageupload-column-common imageupload-column-date', sortable: true, width: 10 },
    { field: 'meta.mod-time', header: 'Mod Time', class: 'imageupload-column-common imageupload-column-date', sortable: true, width: 10 },
    { field: 'status.size', header: 'File Size (bytes)', class: 'imageupload-column-common imageupload-column-filesize', sortable: true, width: 15 },
    { field: 'meta.labels.Version', header: 'Version', class: 'imageupload-column-common imageupload-column-version', sortable: true, width: 15 },
    { field: 'meta.labels', header: 'Details', class: 'imageupload-column-common imageupload-column-labels', sortable: false, width: 40 },
  ];

  exportFilename: string = 'Venice-images';

  constructor(protected controllerService: ControllerService, private objstoreService: ObjstoreService,
    protected cdr: ChangeDetectorRef,
  ) {
    super(controllerService, cdr);
  }

  getClassName(): string {
    return this.constructor.name;
  }
  setDefaultToolbar(): void {
    const breadcrumb = [{ label: 'System Upgrade', url: Utility.getBaseUIUrl() + 'settings/upgrade' }, { label: 'Images', url: '' }];
    const buttons = [];
    if (! this.isToUseWebSocket()) {
      // TODO: delete the refresh button when  watchRolloutImages() is working.  For now, make a refresh button for VS-307
      buttons.push(
        {
          cssClass: 'global-button-primary imageupload-toolbar-button imageupload-toolbar-button-refresh',
          text: 'RERESH',
          callback: () => { this.refreshImages(); },
        }
      );
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: breadcrumb
    });
  }

  postNgInit() {
   if (this.isToUseWebSocket()) {
     this.watchRolloutImages(); // This will fail and trigger "sign out" toaster when using local proxy. // TODO: fix me!
   } else {
      this.getRolloutImages();
   }
  }

  isToUseWebSocket(): boolean {
     // TODO:  in MAC dev env. Use:  return (environment.production) ;
     return true;
  }

  /** Override parent API as watchRolloutImages() is not working. */
  postDeleteRecord() {
      this.refreshImages();
  }

  refreshImages() {
    if (! this.isToUseWebSocket()) {
      this.getRolloutImages();
    }
  }

  /** TODO: This function is not working in local dev using proxy.conf.js . */
  watchRolloutImages() {
    this.rolloutimagesEventUtility = new HttpEventUtility<ObjstoreObject>(ObjstoreObject);
    this.dataObjects = this.rolloutimagesEventUtility.array;
    const sub = this.objstoreService.WatchObject(Utility.ROLLOUT_IMGAGE_NAMESPACE).subscribe(
      response => {
        this.rolloutimagesEventUtility.processEvents(response);
        this.processRolloutImages(response);
      },
    );
    this.subscriptions.push(sub);
  }

  /**
   * 2019-05-15.
   * Once a bundle.tar is uploaded, back-end will un-tar it. Un-taring will have a matadata.json
   * We want take labels out fo metadata.json and add to bundle.tar
   * The image list table will only display bundle.tar
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

  processRolloutImages(response) {
    let metaImage: ObjstoreObject = null;
        let bundleTarImage: ObjstoreObject = null;
        const rolloutImages: IObjstoreObjectList = response.body as IObjstoreObjectList;
        if (rolloutImages.items) {
          const entries = [];
          const allentries = [];
          rolloutImages.items.forEach((image: ObjstoreObject) => {
            if (image.meta.name.endsWith(RolloutUtil.ROLLOUT_METADATA_JSON)) {
              metaImage = image;
            } else if (image.meta.name.endsWith(RolloutUtil.ROLLOUT_BUNDLE_TAR)) {
              bundleTarImage = image;
              entries.push(bundleTarImage);
            }
            allentries.push(image);
          });
          if (bundleTarImage) {  // process only when bundle.tar is available.
            const metaImageLabel: RolloutImageLabel = metaImage.meta.labels as RolloutImageLabel;
            let bundletarImageLabel: RolloutImageLabel = bundleTarImage.meta.labels as RolloutImageLabel;
            if (metaImageLabel) {
              bundletarImageLabel = Utility.getLodash().cloneDeep(metaImageLabel);
            } else {
              const badLabels: RolloutImageLabel = {
                Version: '',
                Description: 'It does not look like a valid image'
              };
              bundletarImageLabel = badLabels;
            }
            bundleTarImage.meta.labels = bundletarImageLabel;
          }
          // in case user uploads a dummy tar file (not bundle.tar), we will show it. So that user can delete it. // 2019-05-17 // TODO: clean it up once back-end is ok.
          this.dataObjects = (entries.length > 0) ? entries : allentries;
        }
  }

  onBeforeSend($event) {
    const xhr = $event.xhr;
    this._xhr = xhr;
    const headerName = AUTH_KEY;
    const token = Utility.getInstance().getXSRFtoken();
    xhr.setRequestHeader(headerName, token);
    this._uploadCancelled = false;
    this.showUploadButton = false;
    this.controllerService.invokeInfoToaster('Upload', 'Please do not move away from this page until upload is completed');
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
    this.controllerService.invokeErrorToaster('Upload failed', 'Involved files: ' + filenames.join(',') + '\n' + JSON.stringify(event.error));
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
  }

  onFileSelect($event) {
    this.fileUploadProgress = 0;
  }

  private resetProgressStatus() {
    if (this.uploadedFiles) {
      this.uploadedFiles.length = 0;
    }
    this.fileUploadProgress = 0;
    this.showUploadButton = true;
    this._xhr = null;
  }

  getFilesNames(files: any[]): string[] {
    const filenames: string[] = [];
    for (const file of files) {
      this.uploadedFiles.push(file);
      filenames.push(file.name);
    }
    return filenames;
  }

  generateDeleteSuccessMsg(object: IObjstoreObject): string {
    return 'Deleted Image ' + object.meta.name;
  }

  deleteRecord(object: IObjstoreObject): Observable<{ body: IObjstoreObject | IApiStatus | Error, statusCode: number }> {
    return this.objstoreService.DeleteObject(Utility.ROLLOUT_IMGAGE_NAMESPACE, object.meta.name);
  }

  generateDeleteConfirmMsg(object: IObjstoreObject) {
    return 'Are you sure you want to delete image: ' + object.meta.name;
  }

  displayColumn(record, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(record, fields);
    const column = col.field;
    switch (column) {
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
      this.fileUploadProgress = $event.progress;
    }
  }

  ngOnDestroy() {
    if (this.isFileUploadInProgress()) {
      this.cancelUpload();
    }
    super.ngOnDestroy();
  }

  private isFileUploadInProgress(): boolean {
    return (this._xhr !== null) || (this.fileUploadProgress > 0) ;
  }
}
