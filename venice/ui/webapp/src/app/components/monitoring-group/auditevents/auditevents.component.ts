import { ChangeDetectorRef, Component, ViewChild, ViewEncapsulation, AfterViewInit, IterableDiffer, IterableDiffers, DoCheck } from '@angular/core';
import { FormArray } from '@angular/forms';
import { MatExpansionPanel } from '@angular/material/expansion';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { GuidedSearchCriteria, SearchGrammarItem, SearchsuggestionTypes } from '@app/components/search/';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { FieldselectorComponent } from '@app/components/shared/fieldselector/fieldselector.component';
import { TableviewAbstract, TablevieweditHTMLComponent, TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ControllerService } from '@app/services/controller.service';
import { SearchService } from '@app/services/generated/search.service';
import { AuditService } from '@app/services/generated/audit.service';
import { LRUMap } from 'lru_map';
import { AuditAuditEvent, IAuditAuditEvent, AuditAuditEvent_outcome } from '@sdk/v1/models/generated/audit';
import { SearchSearchRequest, SearchSearchResponse } from '@sdk/v1/models/generated/search';
import { LazyLoadEvent } from 'primeng/primeng';
import { Observable, Subscription, zip } from 'rxjs';
import { AdvancedSearchComponent } from '@components/shared/advanced-search/advanced-search.component';
import { RowClickEvent, TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { IApiStatus, IMonitoringArchiveQuery, MonitoringArchiveRequest, MonitoringArchiveRequestStatus_status, IMonitoringCancelArchiveRequest } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MonitoringService } from '@app/services/generated/monitoring.service';


@Component({
  selector: 'app-auditevents',
  templateUrl: './auditevents.component.html',
  styleUrls: ['./auditevents.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})

export class AuditeventsComponent extends TablevieweditAbstract<IAuditAuditEvent, AuditAuditEvent> implements AfterViewInit, DoCheck {

  public static AUDITEVENT: string = 'AuditEvent';
  @ViewChild(TablevieweditHTMLComponent) tableWrapper: TablevieweditHTMLComponent;
  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;
  @ViewChild('auditeventSearchPanel') auditeventSearchExpansionPanel: MatExpansionPanel;

  dataObjects: ReadonlyArray<AuditAuditEvent> = [];
  isTabComponent = false;
  disableTableWhenRowExpanded = false;

  // This totalHitRecords indicates the number of records matching searching criteria
  totalHitRecords: number = 0;
  veniceRecords: number = 0;
  displayRecords: number = 0;
  auditEventFieldSelectorOutput: any;

  fieldFormArray = new FormArray([]);
  currentSearchCriteria: string = '';

  exportFilename: string = 'PSM-auditevents';
  numRows: number = 50;

  maxRecords: number = 4000; // AuditEvent query size may cause problem in backend, reduce it to 4000.
  startingSortField: string = 'meta.mod-time';
  startingSortOrder: number = -1;

  loading: boolean = false;
  auditEventDetail: AuditAuditEvent;

  // This will be a map from meta.name to AuditEvent
  cache = new LRUMap<String, AuditAuditEvent>(Utility.getAuditEventCacheSize());  // cache with limit 10

  auditArchiveQuery: IMonitoringArchiveQuery = {}; // TODO: for cancel you can add this line or may be chcek what the advanced search is returning

  lastUpdateTime: string = '';

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/monitoring/ic_audit-black.svg',
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'grid_on'
  };
  // Backend currently only supports time being sorted
  cols: TableCol[] = [
    { field: 'user.name', header: 'Who', class: 'auditevents-column-common auditevents-column-who', sortable: false, width: 10 },
    { field: 'meta.mod-time', header: 'Time', class: 'auditevents-column-common auditevents-column-date', sortable: true, width: '180px' },
    { field: 'action', header: 'Action', class: 'auditevents-column-common auditevents-column-action', sortable: false, width: 9 },
    { field: 'resource.kind', header: 'Act On (kind)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false },
    { field: 'resource.name', header: 'Act On (name)', class: 'auditevents-column-common auditevents-column-act_on', sortable: false, width: 9 },
    { field: 'outcome', header: 'Outcome', class: 'auditevents-column-common auditevents-column-outcome', sortable: false, width: 10 },
    { field: 'client-ips', header: 'Client', class: 'auditevents-column-common auditevents-column-client_ips', sortable: false, width: 17 },
    { field: 'gateway-node', header: 'Service Node', class: 'auditevents-column-common auditevents-column-gateway_node', sortable: false, width: 9 },
    { field: 'service-name', header: 'Service Name', class: 'auditevents-column-common auditevents-column-service_name', sortable: false, width: 14 },
  ];

  exportMap: CustomExportMap = {
    'client-ips': (opts) => {
      const value = Utility.getObjectValueByPropertyPath(opts.data, opts.field);
      return Array.isArray(opts.data) ? value.join(',') : value;
    },
  };

  archiveRequestsAuditUtility: HttpEventUtility<MonitoringArchiveRequest>;
  exportedArchiveRequests: ReadonlyArray<MonitoringArchiveRequest> = [];
  currentArchReqLength: number = 0;
  enableExport: boolean = true;
  displayArchPanel: boolean = false;
  archiveStatusMsg: string = '';
  anyQueryAfterRefresh: boolean = false;
  arrayDiffers: IterableDiffer<any>;

  requestStatus: MonitoringArchiveRequestStatus_status;
  requestName: string = '';
  firstElem: MonitoringArchiveRequest = null;

  constructor(
    protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected searchService: SearchService,
    protected monitoringService: MonitoringService,
    protected auditService: AuditService,
    protected uiconfigsService: UIConfigsService,
    protected iterableDiffers: IterableDiffers
  ) {
    super(controllerService, cdr, uiconfigsService);
    this.arrayDiffers = iterableDiffers.find([]).create(HttpEventUtility.trackBy);
  }

  postNgInit() {
    this.populateFieldSelector();
    this.watchArchiveObject();
  }

  ngAfterViewInit() {
    setTimeout(() => {this.getAuditevents(this.startingSortField, this.startingSortOrder); });
  }

  getClassName(): string {
    return this.constructor.name;
  }

  deleteRecord(object: AuditAuditEvent): Observable<{ body: IAuditAuditEvent | IApiStatus | Error; statusCode: number }> {
    throw new Error('Method not supported.');
  }
  generateDeleteConfirmMsg(object: AuditAuditEvent): string {
    throw new Error('Method not supported.');
  }
  generateDeleteSuccessMsg(object: AuditAuditEvent): string {
    throw new Error('Method not supported.');
  }

  setDefaultToolbar() {
    const buttons = [];
    if (this.checkPermissions()) {
      const exportButton = {
        cssClass: 'global-button-primary global-button-padding',
        genTooltip: () => this.getTooltip(),
        text: 'EXPORT AUDIT EVENTS',
        computeClass: () => this.shouldEnableButtons && this.enableExport ? '' : 'global-button-disabled',
        callback: () => {
          this.createNewObject();
        }
      };
      buttons.push(exportButton);
    }
    this.controllerService.setToolbarData({
      // buttons: [
      //   /*  This block is experimental. debug to "toCSVJSON"
      //   {
      //     cssClass: 'global-button-primary auditevents-toolbar-button',
      //     text: 'To CSV/JSON',
      //     callback: () => { this.toCSVJSON(); },
      //   }
      //   // */
      // ],
      buttons: buttons,
      breadcrumb: [{ label: 'Audit Events', url: Utility.getBaseUIUrl() + 'monitoring/auditevents' }]
    });
  }

  getTooltip() {
    return this.enableExport ? 'Ready to submit archive request' : 'Only ONE archive request can be running at a time';
  }

  /**
   * User will only be allowed to fire only one archive Audit Event request at a time
   * Show the status of the archive request in display panel
   * Allow user to download files on success or cancel when the request is running
   */
  ngDoCheck() {
    const changes = this.arrayDiffers.diff(this.exportedArchiveRequests);
    if (changes) {
      this.handleArchiveLogChange();
    }
  }

  private handleArchiveLogChange() {
    if (this.exportedArchiveRequests.length >= this.currentArchReqLength) {
      this.currentArchReqLength += (this.exportedArchiveRequests.length - this.currentArchReqLength);
      this.firstElem = this.exportedArchiveRequests[0];
      if (this.firstElem.status !== null) {
        if (this.anyQueryAfterRefresh || this.firstElem.status.status === MonitoringArchiveRequestStatus_status.running || this.firstElem.status.status === null) {
          this.displayArchPanel = true;
        }
        this.requestName = this.firstElem.meta.name;
        if (this.firstElem.status.status === MonitoringArchiveRequestStatus_status.running || this.firstElem.status.status === null) {
          // disable the archive request button
          this.enableExport = false;
          this.requestStatus = MonitoringArchiveRequestStatus_status.running;
        } else if (this.firstElem.status.status === MonitoringArchiveRequestStatus_status.completed) {
          // show download link
          this.enableExport = true;
          this.requestStatus = MonitoringArchiveRequestStatus_status.completed;
          // enable the archive request button
        } else {
          this.enableExport = true;
          this.requestStatus = this.firstElem.status.status;
        }
      }
    }
  }

  checkPermissions(): boolean {
    const boolArchiveRequestActions = this.uiconfigsService.isAuthorized(UIRolePermissions['monitoringarchiverequest_all-actions']);
    const boolObjStoreCreate = this.uiconfigsService.isAuthorized(UIRolePermissions.objstoreobject_create);
    return (boolObjStoreCreate && boolArchiveRequestActions);
  }

  /**
   * Cancel any Running Archive Request
   */
  onCancelRecord(event) {
    const object = this.firstElem;
    if (event) {
      event.stopPropagation();
    }
    // Should not be able to cancel any record while we are editing
    if (this.isRowExpanded()) {
      return;
    }
    this.controllerService.invokeConfirm({
      header: this.generateCancelConfirmMsg(object),
      message: 'This action cannot be reversed',
      acceptLabel: 'Cancel Request',
      accept: () => {
        const cancelRequest: IMonitoringCancelArchiveRequest = {
          kind: object.kind,
          'api-version': object['api-version'],
          meta: object.meta
        };
        const sub = this.monitoringService.Cancel(object.meta.name, cancelRequest).subscribe(
          (response) => {
            // TODO: BETTER SOL: From backend if we have some status value saying cancellation in process!
            this.controllerService.invokeSuccessToaster(Utility.CANCEL_SUCCESS_SUMMARY, this.generateCancelSuccessMsg(object));
          },
          (err) => {
            if (err.body instanceof Error) {
              console.error('Service returned code: ' + err.statusCode + ' data: ' + <Error>err.body);
            } else {
              console.error('Service returned code: ' + err.statusCode + ' data: ' + <IApiStatus>err.body);
            }
            this.controllerService.invokeRESTErrorToaster(Utility.CANCEL_FAILED_SUMMARY, err);
          }
        );
        this.subscriptions.push(sub);
      }
    });
  }

  generateCancelSuccessMsg(object: MonitoringArchiveRequest): string {
    return 'Canceled archive request ' + object.meta.name;
  }

  generateCancelConfirmMsg(object: any): string {
    return 'Are you sure to cancel archive request: ' + object.meta.name;
  }

  /**
   * Watch on Archive Audit Event Requests
   * Show any running archive Audit Event request when the user comes on this page
   */
  watchArchiveObject() {
    this.archiveRequestsAuditUtility = new HttpEventUtility<MonitoringArchiveRequest>(MonitoringArchiveRequest);
    this.exportedArchiveRequests = this.archiveRequestsAuditUtility.array;
    const sub = this.monitoringService.WatchArchiveRequest({ 'field-selector': 'spec.type=auditevent' }).subscribe(
      (response) => {
        this.currentArchReqLength = this.exportedArchiveRequests.length;
        this.archiveRequestsAuditUtility.processEvents(response);
        if (this.exportedArchiveRequests.length > 0 && this.exportedArchiveRequests[0].status.status === MonitoringArchiveRequestStatus_status.running) {
          this.enableExport = false;
          this.displayArchPanel = true;
        }
      },
      this.controllerService.webSocketErrorHandler('Failed to get Audit Archive Requests')
    );
    this.subscriptions.push(sub);
  }

  getArchiveQuery(archQuer: IMonitoringArchiveQuery) {
    this.auditArchiveQuery = archQuer;
  }

  toCSVJSON() {
    const csv = Utility.extractTableContentToCSV(this.tableWrapper.table);
    const json = Utility.extractTableContentToJSON(this.tableWrapper.table);
    const csv2json = Utility.csvToObjectArray(csv);
  }

  populateFieldSelector() {
    this.fieldFormArray = new FormArray([]);
  }

  getAuditevents(field = this.tableWrapper.table.sortField,
    order = this.tableWrapper.table.sortOrder) {

    this.loading = true;
    try {
      const searchSearchRequest = this.advancedSearchComponent.getSearchRequest(field, order, 'AuditEvent', false, this.maxRecords);
      this.advancedSearchComponent.emitArchiveQuery();
      this._callSearchRESTAPI(searchSearchRequest);
    } catch (error) {
      this.controllerService.invokeErrorToaster('Input Error', error.toString());
    }
  }

  onTableSort(event: LazyLoadEvent) {
    this.getAuditevents(event.sortField, event.sortOrder);
  }


  /**
   * This serves HTML template
   * @param $event
   */
  handleFieldRepeaterData(values) {
    this.auditEventFieldSelectorOutput = values;
  }

  private _callSearchRESTAPI(searchSearchRequest: SearchSearchRequest) {
    const subscription = this.searchService.PostQuery(searchSearchRequest).subscribe(

      response => {
        const data: SearchSearchResponse = response.body as SearchSearchResponse;
        let objects = data.entries;
        if (!objects || objects.length === 0) {
          this.controllerService.invokeInfoToaster('Information', 'No audit-events found. Please change search criteria.');
          objects = [];
        }
        const entries = [];
        for (let k = 0; k < objects.length; k++) {
          entries.push(objects[k].object); // objects[k] is a SearchEntry object
        }
        // Closing row expand if it is open
        if (this.isRowExpanded()) {
          this.onAuditeventsTableRowClick({
            event: null,
            rowData: this.expandedRowData
          });
        }
        this.lastUpdateTime = new Date().toISOString();
        this.dataObjects = this.mergeEntriesByName(entries);
        // response.body  is like  {total-hits: 430, actual-hits: 430, time-taken-msecs: 100, entries: Array(430)}.
        if (searchSearchRequest !== undefined && searchSearchRequest != null && searchSearchRequest.query != null && searchSearchRequest.query.fields != null && Array.isArray(searchSearchRequest.query.fields.requirements) && searchSearchRequest.query.fields.requirements.length === 0) {
          this.veniceRecords = response.body['total-hits'];
        }
        this.totalHitRecords = response.body['total-hits'];
        this.displayRecords = this.dataObjects.length;
        this.loading = false;
      },
      (error) => {
        this.loading = false;
        this.controllerService.invokeRESTErrorToaster('Failed to get audit-events', error);
      }
    );
    this.subscriptions.push(subscription);
  }

  /**
   * Merge Audit Event by meta.name to remove redundant info
   * It will also preserve uuids in a list for getting request and response
   * @param entries
   */
  mergeEntriesByName(entries: IAuditAuditEvent[]): AuditAuditEvent[] {
    const tmpMap = {};
    entries.forEach(ele => {
      const eleCopy = Utility.getLodash().cloneDeep(ele);
      const key = ele.meta.name;
      if (tmpMap.hasOwnProperty(key)) {
        if (tmpMap[key].outcome === AuditAuditEvent_outcome.failure) {
          eleCopy.outcome = AuditAuditEvent_outcome.failure;
        }
        tmpMap[key] = Utility.getLodash().merge(tmpMap[key], eleCopy);
        tmpMap[key]['uuids'].push(eleCopy.meta.uuid);
      } else {
        tmpMap[key] = eleCopy;
        tmpMap[key]['uuids'] = [eleCopy.meta.uuid];
      }
    });
    return Object.values(tmpMap);
  }

  /**
   * This API serves html template
   */
  displayAuditEvent(): string {
    return JSON.stringify(this.auditEventDetail, null, 1);
  }

  /**
   * Handle logics when user click the row
   * @param event
   */
  onAuditeventsTableRowClick(event: RowClickEvent) {
    if (this.expandedRowData === event.rowData) {
      // Click was on the same row
      this.closeRowExpand();
    } else {
      this.closeRowExpand();
      // fetch detail audit event data
      const auditEvent = event.rowData;
      this.auditEventDetail = this.cache.get(auditEvent.meta.name);  // cache hit
      if (!this.auditEventDetail) {
        // cache miss
        this.fetchAuditEventDetailByUUIDs(auditEvent.uuids, auditEvent, event);
      } else {
        this.expandRowRequest(event.event, event.rowData);
      }
    }
  }

  /**
   * Helper function to fetch audit events by uuids
   * Then, it will merge those audit events as one whole piece with request and reponse object
   * @param uuids
   * @param auditEvent
   * @param event
   */
  fetchAuditEventDetailByUUIDs(uuids: string[], auditEvent: AuditAuditEvent, event: RowClickEvent) {
    this.auditEventDetail = {} as AuditAuditEvent;
    const obs: Observable<any>[] = [];
    uuids.forEach(uuid => {
      const sub = this.auditService.GetGetEvent(uuid);
      obs.push(sub);
    });
    // wait till all observable resolved
    zip(...obs).subscribe(all => {
      this.auditEventDetail = {} as AuditAuditEvent;
      all.forEach(ele => {
        if (this.auditEventDetail.outcome === AuditAuditEvent_outcome.failure) {
          ele.body.outcome = AuditAuditEvent_outcome.failure;
        }
        this.auditEventDetail = Utility.getLodash().merge(this.auditEventDetail, (ele.body as AuditAuditEvent));
      });
      this.cache.set(auditEvent.meta.name, this.auditEventDetail);
      this.expandRowRequest(event.event, event.rowData);
    }, this.controllerService.restErrorHandler('Failed to get Audit Event'));
  }

  /**
   * Override super's API.
   * This api serves html template
   * @param auditevent
   * @param col
   */
  displayColumn(auditevent, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(auditevent, fields);
    const column = col.field;
    switch (column) {
      case 'client-ips':
        return Array.isArray(value) ? value.join(',') : value;
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  /**
   * This API is a helper function. It generates GuidedSearchCriteria from field-selector.
   * @param includeKind
   */
  genGuidedSearchCriteria(includeKind: boolean = true): GuidedSearchCriteria {
    const obj = {
      is: [AuditeventsComponent.AUDITEVENT],
      has: (this.advancedSearchComponent) ? this.advancedSearchComponent.getValues() : []
    };
    if (!includeKind) {
      delete obj.is;
    }
    return obj;
  }

  /**
   * This function builds request json for invoking Search API
   * It should examine the current search context to decide the type of search. (by category, kind, label, fields,etc)
   * @param searched
   */
  protected buildComplexSearchPayload(list: any[], searched: string): any {
    const payload = {
      'max-results': 50,
      'query': {
      }
    };
    // We evaluate the has operations last so that we
    // know if the object kind is an event or not.
    const fieldRequirementIndexes = [];
    for (let i = 0; i < list.length; i++) {
      const obj: SearchGrammarItem = list[i];
      switch (obj.type) {
        case SearchsuggestionTypes.OP_IS:
          payload.query['kinds'] = [AuditeventsComponent.AUDITEVENT];
          break;
        case SearchsuggestionTypes.OP_HAS:
          fieldRequirementIndexes.push(i);
          break;
        default:
          console.error(this.getClassName() + 'buildComplexSearchPayload() does not recognize ' + searched);
      }
    }
    fieldRequirementIndexes.forEach((index) => {
      const obj = list[index];
      const isEvent = payload.query['kinds'] != null && payload.query['kinds'].length === 1 && SearchUtil.isKindInSpecialEventList(payload.query['kinds'][0]);
      payload.query['fields'] = {
        'requirements': SearchUtil.buildSearchFieldsLabelsPayloadHelper(obj, true, isEvent)
      };
    });
    return payload;
  }

  /**
   * This serves HTML API. It clear audit-event search and refresh data.
   * @param $event
   */
  onCancelAuditSearch($event) {
    this.currentSearchCriteria = '';
    this.populateFieldSelector();
    this.getAuditevents();
    this.controllerService.invokeInfoToaster('Infomation', 'Cleared search criteria, audit-events refreshed.');
  }

  /**
   * This is a helper API.  It generates a string representing search criteria.
   */
  buildSearchCriteriaString(): string {
    const guidedSearchCriteria: GuidedSearchCriteria = this.genGuidedSearchCriteria(false);
    return SearchUtil.getSearchInputStringFromGuidedSearchCriteria(guidedSearchCriteria);
  }

}
