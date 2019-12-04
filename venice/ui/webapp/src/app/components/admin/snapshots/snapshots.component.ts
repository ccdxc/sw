import { ChangeDetectorRef, Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ObjstoreService } from '@app/services/generated/objstore.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterConfigurationSnapshotRequest, ClusterSnapshotRestore, IApiStatus, IClusterConfigurationSnapshotRequest, IClusterSnapshotRestore, IClusterConfigurationSnapshot, ClusterConfigurationSnapshot } from '@sdk/v1/models/generated/cluster';
import { IObjstoreObject, IObjstoreObjectList, ObjstoreObject } from '@sdk/v1/models/generated/objstore';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { Observable } from 'rxjs';
import { AUTH_KEY } from '@app/core';

/**
 * This component let user run CRUD operations on snapshot configurations.
 *
 * 1. When page starts up, UI will check if Venice has ClusterConfigurationSnapshot object. If not, creates it.
 * 2. List all snapshots by fetching them from objectstore.  see getSnapshots()
 * 3. User can run save snapshot and restore snapshot.
 *
 */

@Component({
  selector: 'app-snapshots',
  templateUrl: './snapshots.component.html',
  styleUrls: ['./snapshots.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class SnapshotsComponent extends TablevieweditAbstract<IObjstoreObject, ObjstoreObject> implements OnInit {
  public static SNAPSHOT_UI_FIELD_DOWNLOADURL: string = 'downloadurl';
  public static SNAPSHOT_NAMESPACES: string = 'snapshots';
  public static SNAPSHOT_RESTORE_METANAME: string = 'SnapshotRestore';

  dataObjects: ReadonlyArray<ObjstoreObject> = [];

  /** file upload variables */
  uploadedFiles: any[] = [];
  uploadingFiles: string[] = [];
  fileUploadProgress: number = 0;

  _uploadCancelled: boolean = false;
  showUploadButton: boolean = true;
  uploadInForeground: boolean = false;
  _xhr: XMLHttpRequest = null;
  /** file upload variables */

  bodyicon: Icon = {
    margin: {
      top: '9px',
      left: '8px',
    },
    matIcon: 'restore'
  };
  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'snapshots-column-name', sortable: true, width: 45, notReorderable: true },
    { field: 'meta.mod-time', header: 'Time', class: 'snapshots-column-date', sortable: true, width: '200px', notReorderable: true },
    { field: 'status.size', header: 'File Size', class: ' snapshots-column-filesize', sortable: true, width: 15 }
  ];

  exportFilename: string = 'Venice-configuration-snapshots';

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  shouldEnable_takesnapshot_button: boolean = true;
  shouldEnable_refresh_button: boolean = true;
  exportMap: CustomExportMap = {};

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    private objstoreService: ObjstoreService,
    protected clusterService: ClusterService) {

    super(controllerService, cdr, uiconfigsService);
  }

  postNgInit(): void {
    this.checkAndMakeSnapshotPolicy();
    this.getSnapshots();
  }


  setDefaultToolbar() {
    const buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.objstorebucket_read)) {
      const refreshButton = {
        cssClass: 'global-button-primary snapshots-toolbar-button snapshots-toolbar-button-ADD',
        text: 'REFRESH',
        computeClass: () => this.shouldEnable_refresh_button ? '' : 'global-button-disabled',
        callback: () => { this.refresh(); }
      };
      if (this.uiconfigsService.isAuthorized(UIRolePermissions.objstorebucket_create) &&
          this.uiconfigsService.isAuthorized(UIRolePermissions.clustersnapshotrestore_create)) {
        const saveButton = {
          cssClass: 'global-button-primary snapshots-toolbar-button snapshots-toolbar-button-ADD',
          text: 'SAVE A CONFIG SNAPSHOT',
          computeClass: () => this.shouldEnable_takesnapshot_button ? '' : 'global-button-disabled',
          callback: () => { this.saveConfigSnapshot(); }
        };
        buttons.push(saveButton);
      }
      buttons.push(refreshButton);
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Snapshots', url: Utility.getBaseUIUrl() + 'admin/snapshots' }]
    });
  }
  refresh() {
    this.getSnapshots();
  }

  saveConfigSnapshot() {
    const clusterConfigurationSnapshotRequest: IClusterConfigurationSnapshotRequest = new ClusterConfigurationSnapshotRequest();
    this.clusterService.Save(clusterConfigurationSnapshotRequest).subscribe(
      (response) => {
        this.controllerService.invokeSuccessToaster('Success.', 'Saved configuration snapshot.' );
        this.refresh();
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster('Failed to save config snapshot', error);
      }
    );
  }

  processSnapshotImages(response: any) {
    let snapshotImages: IObjstoreObjectList = null;
    snapshotImages = (response) ? response.body as IObjstoreObjectList : snapshotImages;
    if (snapshotImages && snapshotImages.items) {
      const entries = [];
      const allentries = [];
      snapshotImages.items.forEach((snapshotImage: ObjstoreObject) => {
        snapshotImage[SnapshotsComponent.SNAPSHOT_UI_FIELD_DOWNLOADURL] = this.buildSnapshotImageDownloadURLHelper(snapshotImage.meta.name);
        entries.push(snapshotImage);
      });
      this.dataObjects = (entries.length > 0) ? entries : [];
    } else {
      this.dataObjects = []; // it possible that server has no snapshot images.
    }
  }

  /**
   * Snapshot download image url is like
   * https://10.7.100.21/objstore/v1/downloads/snapshots/backup2019-11-12T01-14-00Z
   * @param instanceId
   */
  buildSnapshotImageDownloadURLHelper(instanceId: string): any {
    return '/objstore/v1/downloads/snapshots/' + instanceId;
  }

  /**
   * This API check if backend already has Snapshot config policy.
   * If not, create one.  // Per VS-904
   */
  checkAndMakeSnapshotPolicy() {
    const sub = this.clusterService.GetConfigurationSnapshot().subscribe(
      (response) => {
          //  We found existing Snapshot config.  So do nonthing.
      },
      (error) => {
        console.error(this.getClassName() +  'checkAndMakeSnapshotPolicy(). Found not snapshot policy. Create one');
        this.generateConfigSnapshot();
      }
    );
    this.subscriptions.push(sub);
  }

  /**
   * ClusterConfigurationSnapshot is a singleton object in Venice.
   * At 2019-11-26, we only have one configuration as below. It means configure snapshot will be saved in objectstore.
   */
  generateConfigSnapshot() {
    const config =   {
      'kind': 'ConfigSnapshot',
      'meta': {
        'name': 'GlobalSnapshot'
      },
      'spec': {
        'periodicity': '2h',
        'destination': {
          'Type': 'objectstore',
          'object-store': {
            'bucket': 'snapshots'
          }
        }
      }
    };
    const myConfigurationSnapshot: IClusterConfigurationSnapshot = new ClusterConfigurationSnapshot(config);
    const sub = this.clusterService.AddConfigurationSnapshot(myConfigurationSnapshot).subscribe(
      (response) => {
          //  We found existing Snapshot config.  So do nonthing.
      },
      (error) => {
        this.controllerService.invokeErrorToaster('Creating Snapshot Failed', 'Failed to generate Global Configuration Policy, Please contact system administrator.');
      }
    );
    this.subscriptions.push(sub);
  }


  getSnapshots() {
    const sub = this.objstoreService.ListObject(SnapshotsComponent.SNAPSHOT_NAMESPACES).subscribe(
      (response) => {
        this.processSnapshotImages(response);
      },
      this.controllerService.restErrorHandler('Failed to fetch snapshot images.')
    );
    this.subscriptions.push(sub);
  }

  deleteRecord(object: ObjstoreObject): Observable<{ body: ObjstoreObject | IApiStatus | Error, statusCode: number }> {
    return this.objstoreService.DeleteObject(SnapshotsComponent.SNAPSHOT_NAMESPACES, object.meta.name);
  }

  postDeleteRecord() {
    this.refresh();
  }



  generateDeleteConfirmMsg(object: ObjstoreObject): string {
    return 'Are you sure you want to delete cluster configuration snapshot: ' + object.meta.name;

  }
  generateDeleteSuccessMsg(object: ObjstoreObject): string {
    return 'Deleted cluster configuration snapshot ' + object.meta.name;

  }

  /**
   * This API serves html template
   * @param rowData
   */
  showDeleteIcon(rowData: ObjstoreObject): boolean {
    return true;
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return TableUtility.displayColumn(data, col);
    }
  }


  invokeRestore(rowData: ObjstoreObject) {
    const clusterSnapshotRestore: IClusterSnapshotRestore = new ClusterSnapshotRestore();
    clusterSnapshotRestore.meta.name = SnapshotsComponent.SNAPSHOT_RESTORE_METANAME;
    clusterSnapshotRestore.spec.path = rowData.meta.name;

    const sub = this.clusterService.Restore(clusterSnapshotRestore).subscribe(
      (response) => {
        this.controllerService.invokeSuccessToaster('Invoked config restore.', 'Venice will be unavailable. You will be logged out. Please refresh browser to login again.');
        const setTime1 = window.setTimeout(() => {
          this._controllerService.publish(Eventtypes.LOGOUT, { 'reason': 'Restoring Venice configuration.' });
          window.clearTimeout(setTime1);
        }, 3000);
      },
      (error) => {
        if (error.body instanceof Error) {
          console.error('Service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.error('Service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
        this.controllerService.invokeRESTErrorToaster('Restore Failed', error);
      }
    );
    this.subscriptions.push(sub);
  }

  onRestoreSnapshotClick($event, rowData: ObjstoreObject) {
    this.controllerService.invokeConfirm({
      header: 'Are you sure?',
      message: 'This action cannot be reversed',
      acceptLabel: 'Restore',
      accept: () => {
        this.invokeRestore(rowData);
      }
    });
  }

  /** File upload API  START */


  isThereBackgroudFileUpload() {
    return false;
  }
  /**
  * This API serves html template
  */
  onUpload(event) {
    const files = event.files;
    const filenames = this.getFilesNames(files);
    this.controllerService.invokeInfoToaster('Upload succeeded', 'Uploaded files: ' + filenames.join(',') + '. System will validate uploaded files.');
    this.refresh();
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
  onCancelUpload() {
    if (this.isFileUploadInProgress()) {
      this.cancelUpload();  // cancel upload only when upload started.
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

  isFileUploadInProgress(): boolean {
    return (this._xhr !== null) || (this.fileUploadProgress > 0);
  }

  onFileSelect($event) {
    // Just uploading one file.
    if ($event.files.length > 0) {
      this.uploadingFiles.push($event.files[0]['name']);
    }
    this.fileUploadProgress = 0;
  }

  resetProgressStatus() {
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

  onUploadProgress($event) {
    if (!this._uploadCancelled) {
      const progress = $event.progress;
      this.fileUploadProgress = (progress > this.fileUploadProgress) ? progress : this.fileUploadProgress;
    }
  }

  /** File upload API  END */
}
