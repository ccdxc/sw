
import { Observable, forkJoin } from 'rxjs';
import { Utility } from '@app/common/Utility';
import { ControllerService } from '@app/services/controller.service';
import { LocalSearchRequest, AdvancedSearchComponent } from '@app/components/shared/advanced-search/advanced-search.component';
import { FieldsRequirement, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';
import { Output, EventEmitter, ViewChild, OnInit } from '@angular/core';
import { TableCol } from '@app/components/shared/tableviewedit';
import { StagingService } from '@app/services/generated/staging.service';
import { StagingCommitAction, StagingBuffer, IBulkeditBulkEditItem, IStagingBulkEditAction } from '@sdk/v1/models/generated/staging';
import { switchMap, buffer } from 'rxjs/operators';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { BaseComponent } from '@app/components/base/base.component';

export abstract class DataComponent extends BaseComponent implements OnInit {
  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;

  @Output() operationOnMultiRecordsComplete: EventEmitter<any> = new EventEmitter<any>();

  protected stagingService: StagingService;

  constructor(protected controllerService: ControllerService,
      protected uiconfigsService: UIConfigsService) {
      super(controllerService, uiconfigsService);
      this.setupBulkEdit();
  }


  /**
   * Call this from ngOnInit in derived class: tableviewedit or pentable
   */
  setupBulkEdit() {
      this.stagingService = Utility.getInstance().getStagingServices();
  }

  abstract getSelectedDataObjects(): any[];

  hasSelectedRows(): boolean {
    return this.getSelectedDataObjects().length > 0;
  }

  commitStagingBuffer(buffername: string): Observable<any> {
      const commitBufferBody: StagingCommitAction = Utility.buildCommitBufferCommit(buffername);
      return this.stagingService.Commit(buffername, commitBufferBody);
  }

  createStagingBuffer(): Observable<any> {
    const stagingBuffer: StagingBuffer = Utility.buildCommitBuffer();
    return this.stagingService.AddBuffer(stagingBuffer);
  }

  deleteStagingBuffer(buffername: string, reason: string, isToshowToaster: boolean = false) {
    if (buffername == null) {
      return;
    }
    this.stagingService.DeleteBuffer(buffername).subscribe(
      response => {
        if (isToshowToaster) {
          this._controllerService.invokeSuccessToaster('Bulkedit commit ', 'Deleted Buffer ' + buffername + '\n' + reason);
        }
      },
      this._controllerService.restErrorHandler('Delete Staging Buffer Failed')
    );
  }

  /**
   * This API builds an IBulkeditBulkEditItem
   */
  buildBulkEditItemFromVeniceObject(vObject: any, method: string, uri: string = ''): IBulkeditBulkEditItem {
    const obj = {
      uri: uri,
      method: method,
      object: this.trimObjectForBulkeditItem(vObject, vObject, null, true)
    };
    return obj as IBulkeditBulkEditItem;
  }
  /**
   * This API builds an IBulkeditBulkEditItem
   *
   */
  trimObjectForBulkeditItem(vObject: any, model, previousVal = null, trimDefaults = true ): any {
    return vObject;
  }

  /**
   * This API builds an IStagingBulkEditAction
   * veniceObjects can be ClusterDistributedServiceCard[] or ClusterNetworkInterface[]. // Any Venice object
   * @param veniceObjects
   * @param buffername
   * @param method
   */
  buildBulkEditItemPayload(veniceObjects: any[], buffername: string = '', method: string = 'update') {
    const stagingBulkEditAction: IStagingBulkEditAction = Utility.buildStagingBulkEditAction(buffername);
    stagingBulkEditAction.spec.items = [];

    for (const vObject of veniceObjects) {
      const bulkeditBulkEditItem: IBulkeditBulkEditItem = this.buildBulkEditItemFromVeniceObject(vObject, method);
      stagingBulkEditAction.spec.items.push(bulkeditBulkEditItem);
    }
    return stagingBulkEditAction;
  }

  /**
   * This API builds an IStagingBulkEditAction
   * veniceObjects can be ClusterDistributedServiceCard[] or ClusterNetworkInterface[]. // Any Venice object
   * It is used in DSCs, NetworkInterfaces pages, etc to update venice-objects meta.labels
   * @param veniceObjects
   * @param buffername
   */
  buildBulkEditLabelsPayload(veniceObjects: any[], buffername: string = ''): IStagingBulkEditAction {
    return this.buildBulkEditItemPayload(veniceObjects, buffername, 'update');
  }

  /**
   * Override-able api
   * It is used when buledit commit buffer is done successfully.
   */
  onBulkEditSuccess(veniceObjects: any[], stagingBulkEditAction: IStagingBulkEditAction, successMsg: string, failureMsg: string) {
    // do nothing
  }

  /**
   * Override-able api
   * It is used when buledit commit buffer failed.
   */
  onBulkEditFailure(error: Error, veniceObjects: any[], stagingBulkEditAction: IStagingBulkEditAction, successMsg: string, failureMsg: string ) {
    // doing nothing
    // By looking into parameters, one can find out the operation context.
  }

    /**
   * This API verify if (about to commit) buffer content match bulkedit request and original user request
   * @stagingBulkEditAction is original user input
   * @bulkeditResponse has bulkedit content
   * @getbufferResponse buffer content
   *
   * One can override this API to customize verification logic
   * In case, there are multiple bulkedi operations in one component
   * stagingBulkEditAction can tell the differences of bulkedit operations
   *
   */
  verifybulkEditBufferContent(stagingBulkEditAction: IStagingBulkEditAction, bulkeditResponse: any, getbufferResponse: any ): boolean {
    try {
       return  (stagingBulkEditAction.spec.items.length  === getbufferResponse.body.status.items.length &&  getbufferResponse.body.status.items.length === bulkeditResponse.body.status.items.length);
    } catch (error) {
      console.error(this.getClassName() + ' .verifybulkEditBufferContent() ' + error);
      return false;
    }

  }

  /**
   * This API builds an IStagingBulkEditAction
   * veniceObjects can be ClusterDistributedServiceCard[] or ClusterNetworkInterface[]. // Any Venice object
   * It is used in DSCs, NetworkInterfaces pages, etc to update venice-objects meta.labels
   * @param veniceObjects
   * @param buffername
   */
  buildBulkEditPayloadHelper(veniceObjects: any[], method: string,  buffername: string = ''): IStagingBulkEditAction {
    const stagingBulkEditAction: IStagingBulkEditAction = Utility.buildStagingBulkEditAction(buffername);
    stagingBulkEditAction.spec.items = [];
    for (const vObject of veniceObjects) {
      const bulkeditBulkEditItem: IBulkeditBulkEditItem = this.buildBulkEditItemFromVeniceObject(vObject, method);
      stagingBulkEditAction.spec.items.push(bulkeditBulkEditItem);
    }
    return stagingBulkEditAction;
  }

  /**
   * This API build bulkedit delete payload
   * @param veniceObjects
   * @param buffername
   */
  buildBulkEditDeletePayload(veniceObjects: any[], buffername: string = ''): IStagingBulkEditAction {
    return this.buildBulkEditPayloadHelper(veniceObjects, 'delete', buffername);
  }

  invokeDeleteMultipleRecordsBulkedit(successMsg: string, failureMsg: string) {
    const selectedDataObjects = this.getSelectedDataObjects();
    const stagingBulkEditAction = this.buildBulkEditDeletePayload(selectedDataObjects);
    this.bulkEditHelper(selectedDataObjects, stagingBulkEditAction, successMsg, failureMsg);
  }

  /**
   * This API is used in html template. P-table with checkbox enables user to select multiple records. User can delete multiple records.
   * This function asks for user confirmation and invokes the REST API.
   */
  onDeleteSelectedRows($event) {
    const selectedDataObjects = this.getSelectedDataObjects();
    this.controllerService.invokeConfirm({
      header: 'Delete selected ' + selectedDataObjects.length + ' records?',
      message: 'This action cannot be reversed',
      acceptLabel: 'Delete',
      accept: () => {
        if (this.getSelectedDataObjects().length <= 0) {
          return;
        }

        const allSuccessSummary = 'Deleted';
        const partialSuccessSummary = 'Partially deleted';
        const msg = 'Deleted ' + selectedDataObjects.length + ' selected records.';

        const successMsg = 'Successfully deleted  ' + selectedDataObjects.length + ' records.';
        const failureMsg = 'Failed to delete ' + selectedDataObjects.length + ' records.';
        this.invokeDeleteMultipleRecordsBulkedit(successMsg, failureMsg);  // use bulkedit
      }
    });
  }

  // TODO: This API shouldn't be needed once bulk edit is used. Keeping it for now.

  /**
   * This api use forkJoin technique to update multiple records
   * @param observables
   * @param allSuccessSummary
   * @param partialSuccessSummary
   * @param msg
   */
  invokeAPIonMultipleRecords(observables: Observable<any>[] = [], allSuccessSummary: string, partialSuccessSummary: string, msg: string,
    successCallback: () => void = null, errorCallback: (error: any) => void = null
  ) {
    if (observables.length <= 0) {
      return;
    }
    const sub = forkJoin(observables).subscribe(
      (results) => {
        this.operationOnMultiRecordsComplete.emit(results);
        const isAllOK = Utility.isForkjoinResultAllOK(results);
        if (isAllOK) {
          this.controllerService.invokeSuccessToaster(allSuccessSummary, msg);
          if (successCallback) {
            successCallback();
          }
        } else {
          const error = Utility.joinErrors(results);
          this.controllerService.invokeRESTErrorToaster(partialSuccessSummary, error);
          if (errorCallback) {
            errorCallback(error);
          }
        }
      },
      (error) => {
        this.operationOnMultiRecordsComplete.emit(error);
        this.controllerService.invokeRESTErrorToaster('Failure', error);
        if (errorCallback) {
          errorCallback(error);
        }
      }
    );
    this.subscriptions.push(sub);
  }
  /**
   * This API perform bulkedit call.
   * @param veniceObjects
   * @param stagingBulkEditAction
   * @param successMsg
   * @param failureMsg
   * @param successTitle
   * @param failureTitle
   *
   * Per VS-1530,
   *    0. prepare data
   *    1. create staging buffer  (AAA)
   *    2. invoke bulkedit    (using ASS)
   *    3. get buffer  (fetch content of AAA)
   *    4. verify (0, 1, 2 are matching)
   *    5. commit buffer (commit AAA)
   *    6. finally, delete buffer (delete AAA) to release server resource
  */
  bulkEditHelper(veniceObjects: any[], stagingBulkEditAction: IStagingBulkEditAction, successMsg: string, failureMsg: string, successTitle: string = Utility.UPDATE_SUCCESS_SUMMARY, failureTitle: string = Utility.UPDATE_FAILED_SUMMARY) {
    if (veniceObjects.length <= 0) {
      return;
    }
    let createdBuffer: StagingBuffer = null;  // responseBuffer.body as StagingBuffer;
    let buffername = null; // createdBuffer.meta.name;
    this.createStagingBuffer().pipe(   // (A) create buffer
      switchMap(responseBuffer => {
        createdBuffer = responseBuffer.body as StagingBuffer;
        buffername = createdBuffer.meta.name;
        stagingBulkEditAction.meta.name = buffername;  // make sure to set buffer-name to stagingBulkEditAction
        return  this.stagingService.Bulkedit(buffername, stagingBulkEditAction, null, false, false).pipe(
          // third parameter has to be 'null'. o.w, REST API url will be screwed up
         switchMap( (bulkeditResponse) => {
           return this.stagingService.GetBuffer(buffername).pipe(
             switchMap( getbufferResponse => {
               if (this.verifybulkEditBufferContent(stagingBulkEditAction, bulkeditResponse, getbufferResponse)) {
                   return this.commitStagingBuffer(buffername); //  commit buffer
               } else {
                  console.error(this.getClassName() + ' bulkEditHelper() verifybulkEditBufferContent() failed ' , stagingBulkEditAction, bulkeditResponse,  getbufferResponse ) ;
                  const error = new Error('Failed to verify commit buffer content');
                  this.onBulkEditFailure(error, veniceObjects, stagingBulkEditAction, successMsg, failureMsg + ' ' + error.toString());
                  throw error;
              }
             })
           );
         }),
        );
      })
    ).subscribe(
      (responseCommitBuffer) => {
        this._controllerService.invokeSuccessToaster(successTitle, successMsg);
        this.onBulkEditSuccess(veniceObjects, stagingBulkEditAction, successMsg, failureMsg);
        this.deleteStagingBuffer(buffername, failureMsg, false); // if bulked is successful  just delete tbe buffer to release resource
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster(failureTitle, error);
        this.onBulkEditFailure(error, veniceObjects, stagingBulkEditAction, successMsg, failureMsg);
        this.deleteStagingBuffer(buffername, failureMsg, false); // just delete tbe buffer to release resource
      }
    );
  }

  formatLabels(labelObj) {
      const labels = [];
      if (labelObj != null) {
          Object.keys(labelObj).forEach((key) => {
          labels.push(key + ': ' + labelObj[key]);
          });
      }
      return labels.join(', ');
  }

  // TODO: Add updatewithforkjoin

  /**
   * This is API execute table search. See how Hosts and Workload components use it.
   * @param field
   * @param order
   * @param kind
   * @param maxSearchRecords
   * @param advSearchCols
   * @param dataObjects
   * @param isShowToasterOnSearchHasResult default is false.
   * @param isShowToasterOnSearchNoResult default is true.
   */
  onSearchDataObjects(field, order, kind: string, maxSearchRecords, advSearchCols: TableCol[], dataObjects, advancedSearchComponent?: AdvancedSearchComponent,
    isShowToasterOnSearchHasResult: boolean = false,
    isShowToasterOnSearchNoResult: boolean = true): any[] | ReadonlyArray<any> {
    try {
      const searchComponent = advancedSearchComponent;
      const searchSearchRequest = searchComponent.getSearchRequest(field, order, kind, true, maxSearchRecords);
      const localSearchRequest: LocalSearchRequest = searchComponent.getLocalSearchRequest(field, order);
      const requirements: FieldsRequirement[] = (searchSearchRequest.query.fields.requirements) ? searchSearchRequest.query.fields.requirements : [];
      const localRequirements: FieldsRequirement[] = (localSearchRequest.query) ? localSearchRequest.query as FieldsRequirement[] : [];

      const searchTexts: SearchTextRequirement[] = searchSearchRequest.query.texts;
      const searchResults = TableUtility.searchTable(requirements, localRequirements, searchTexts, advSearchCols, dataObjects); // Putting this.dataObjects here enables search on search. Otherwise, use this.dataObjectsBackup
      if (isShowToasterOnSearchNoResult && (!searchResults || searchResults.length === 0)) {
        this.controllerService.invokeInfoToaster('Information', 'No ' + kind + ' records found. Please change search criteria.');
      } else {
        if (isShowToasterOnSearchHasResult) {
          this.controllerService.invokeInfoToaster('Information', 'Found ' + searchResults.length + ' ' + kind + ' records');
        }
      }
      return searchResults;
    } catch (error) {
      this.controllerService.invokeErrorToaster('Error', error.toString());
      return [];
    }
  }
}
