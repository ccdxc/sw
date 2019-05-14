import { Component, OnInit, ViewEncapsulation, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { Observable } from 'rxjs';

import { ControllerService } from '@app/services/controller.service';
import { TableCol, TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { IApiStatus, IObjstoreObject, ObjstoreObject, IObjstoreObjectList, ObjstoreObjectList } from '@sdk/v1/models/generated/objstore';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Utility } from '@app/common/Utility';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { AUTH_KEY } from '@app/core';

/**
 * This component let user upload Venice rollout images and manage existing images.
 * Note:
 *   imageupload.component.html use <p-fileUpload> widget which not using Angular http-clinet. We have to use p-fileUpload.onBeforeSend() hook to add security-token.
 *
 */
@Component({
  selector: 'app-imageupload',
  templateUrl: './imageupload.component.html',
  styleUrls: ['./imageupload.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})

export class ImageuploadComponent extends TablevieweditAbstract<IObjstoreObject, ObjstoreObject> implements OnInit {

  dataObjects: ReadonlyArray<ObjstoreObject> = [];
  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = true;
  rolloutimagesEventUtility: HttpEventUtility<ObjstoreObject>;
  loading: boolean = false;

  uploadedFiles: any[] = [];
  fileUploadProgress: number = 0;

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

  constructor(protected controllerService: ControllerService, private objstoreService: ObjstoreService,
    protected cdr: ChangeDetectorRef,
  ) {
    super(controllerService, cdr);
  }

  getClassName(): string {
    return this.constructor.name;
  }
  setDefaultToolbar(): void {
    this.controllerService.setToolbarData({
      buttons: [
      ],
      breadcrumb: [{ label: 'System Upgrade', url: Utility.getBaseUIUrl() + 'settings/upgrade' }, { label: 'Images', url: '' }]
    });
  }

  postNgInit() {
    // this.watchRolloutImages();
    this.getRolloutImages();
  }

  /** Override parent API as watchRolloutImages() is not working. */
  postDeleteRecord() {
    this.refreshImages();
  }

  /** TODO: delete me once watchRolloutImages() is working */
  refreshImages() {
    this.getRolloutImages();
  }

  /** TODO: This function is not working yet. */
  watchRolloutImages() {
    this.rolloutimagesEventUtility = new HttpEventUtility<ObjstoreObject>(ObjstoreObject);
    this.dataObjects = this.rolloutimagesEventUtility.array;
    const sub = this.objstoreService.WatchObject(Utility.ROLLOUT_IMGAGE_NAMESPACE).subscribe(
      response => {
        this.rolloutimagesEventUtility.processEvents(response);
      },
    );
    this.subscriptions.push(sub);
  }

  getRolloutImages() {
    const sub = this.objstoreService.ListObject(Utility.ROLLOUT_IMGAGE_NAMESPACE).subscribe(
      (response) => {
        const rolloutImages: IObjstoreObjectList = response.body as IObjstoreObjectList;
        if (rolloutImages.items) {
          const entries = [];
          rolloutImages.items.forEach(image => {
            entries.push(image);
          });
          this.dataObjects = entries;
        }
      },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Failed to fetch rollout images.', error);
      }
    );
    this.subscriptions.push(sub);
  }

  onBeforeSend($event) {
    const xhr = $event.xhr;
    const headerName = AUTH_KEY;
    const token = Utility.getInstance().getXSRFtoken();
    xhr.setRequestHeader(headerName, token);
  }
  /**
   * This API serves html template
   */
  onUpload(event) {
    const files = event.files;
    const filenames = this.getFilesNames(files);
    this.controllerService.invokeInfoToaster('Upload succeeded', 'Uploaded files: ' + filenames.join(',') + '. System will validate uploaded files.');
    this.refreshImages();
    this.fileUploadProgress = 0;
  }

  /**
  * This API serves html template
  */
  onError(event) {
    const files = event.files;
    const filenames = this.getFilesNames(files);
    this.controllerService.invokeErrorToaster('Upload failed', 'Involved files: ' + filenames.join(',') + '\n' + JSON.stringify(event.error));
    this.fileUploadProgress = 0;
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
    this.fileUploadProgress = $event.progress;
  }
}
