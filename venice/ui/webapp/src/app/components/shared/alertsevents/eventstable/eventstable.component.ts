import { Component, OnInit, Input, ChangeDetectorRef, SimpleChanges, OnChanges, OnDestroy, ViewEncapsulation, ViewChild, IterableDiffer, IterableDiffers, AfterViewInit, DoCheck } from '@angular/core';
import { Observable, forkJoin, throwError, Subscription } from 'rxjs';
import { EventsEvent_severity, EventsEventAttributes_severity, IApiListWatchOptions, IEventsEvent, EventsEvent, ApiListWatchOptions_sort_order, EventsEventList } from '@sdk/v1/models/generated/events';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { EventsService } from '@app/services/events.service';
import { FormControl, FormArray } from '@angular/forms';
import { FieldsRequirement, FieldsRequirement_operator, ISearchSearchResponse, SearchSearchRequest, SearchTextRequirement, SearchSearchResponse } from '@sdk/v1/models/generated/search';
import { SearchService } from '@app/services/generated/search.service';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ControllerService } from '@app/services/controller.service';
import { debounceTime, distinctUntilChanged, switchMap, first } from 'rxjs/operators';
import { AlertsEventsSelector } from '@app/components/shared/alertsevents/alertsevents.component';
import { IApiStatus } from '@sdk/v1/models/generated/search';
import { Animations } from '@app/animations';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { AdvancedSearchComponent } from '../../advanced-search/advanced-search.component';
import { IMonitoringArchiveQuery, MonitoringArchiveRequest, MonitoringArchiveRequestStatus_status, IMonitoringCancelArchiveRequest } from '@sdk/v1/models/generated/monitoring';
import { ExportLogsComponent } from '../../exportlogs/exportlogs.component';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  selector: 'app-eventstable',
  templateUrl: './eventstable.component.html',
  styleUrls: ['./eventstable.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventstableComponent extends TablevieweditAbstract<IEventsEvent, EventsEvent> implements OnChanges, OnDestroy, AfterViewInit, DoCheck {

  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;
  @ViewChild('exportLogsComponent') exportLogsComponent: ExportLogsComponent;

  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = false;
  exportFilename: string = 'PSM-securityapp';

  // Used as the key for uniquely identifying poll requests.
  // If there are going to be two of these components alive at the same time,
  // this field is required for them to have independent queries.
  @Input() pollingServiceKey: string = 'alertsevents';
  // If provided, will only show alerts and events
  // where the source node matches
  @Input() selector: AlertsEventsSelector;
  @Input() showEventsAdvSearch: boolean = false;
  @Input() searchedEvent: string;

  currentEventSeverityFilter = null;
  showDebugEvents: boolean = false;
  eventsSubscription: Subscription;
  eventSearchFormControl: FormControl = new FormControl();
  severityEnum = EventsEventAttributes_severity;
  eventFieldSelectorOutput: any;
  currentSearchCriteria: string = '';
  fieldFormArray = new FormArray([]);
  eventArchiveQuery: IMonitoringArchiveQuery = {};

  // EVENTS
  // Used for the table - when true there is a loading icon displayed
  eventsLoading = false;

  // Holds all events
  events: EventsEvent[] = [];
  // Contains the total number of events.
  // Count does not include debug events if show debug events is not selected
  eventsTotalCount = 0;

  // Mapping from meta.name to event object. Used for mapping entries from elastic
  // to the event objects we have.
  eventMap = {};

  // holds a subset (possibly all) of this.events
  // This are the events that will be displayed
  dataObjects: EventsEvent[] = [];
  persistentEvents: EventsEvent[] = [];

  eventsPostBody: IApiListWatchOptions = { 'sort-order': ApiListWatchOptions_sort_order.none };

  // we only fetch the past 1000 events, so we don't know if there are more in the
  // system for our search criteria
  eventLimit: number = 1000;

  // All columns are set as not sortable as it isn't currently supported
  // TODO: Support sorting columns
  cols: TableCol[] = [
    { field: 'severity', header: 'Severity', class: '', sortable: false, width: 10 },
    { field: 'type', header: 'Type', class: '', sortable: true, width: 8 },
    { field: 'message', header: 'Message', class: '', sortable: false, width: 30 },
    { field: 'object-ref', header: 'Object Ref', class: '', sortable: false, width: 10 },
    { field: 'count', header: 'Count', class: '', sortable: false, width: 5 },
    { field: 'source', header: 'Source Node & Component', class: '', sortable: false},
    { field: 'meta.mod-time', header: 'Time', class: '', sortable: true, width: 13 }
  ];

  // Will hold mapping from severity types to counts
  eventNumbers: { [severity in EventsEvent_severity]: number } = {
    'info': 0,
    'warn': 0,
    'critical': 0,
    'debug': 0
  };

  eventsIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'event'
  };

  // TimeRange for events and alerts
  eventsSelectedTimeRange: TimeRange;
  eventsTimeConstraints: string = '';

  exportMap: CustomExportMap = {};

  selectedEvent: EventsEvent = null;
  maxRecords: number = 4000;

  startingSortField: string = 'meta.mod-time';
  startingSortOrder: number = -1;

  displayArchPanel: boolean = false;
  archiveRequestsEventUtility: HttpEventUtility<MonitoringArchiveRequest>;
  exportedArchiveRequests: ReadonlyArray<MonitoringArchiveRequest> = [];
  currentArchReqLength: number = 0;
  archiveStatusMsg: string = '';
  anyQueryAfterRefresh: boolean = false;
  enableExport: boolean = true;

  arrayDiffers: IterableDiffer<any>;
  requestStatus: MonitoringArchiveRequestStatus_status;
  requestName: string = '';
  firstElem: MonitoringArchiveRequest = null;
  advSearchCols: TableCol[] = [];
  archivePermissions: boolean = true;
  eventsTimeBased: EventsEvent[] = [];

  constructor(protected eventsService: EventsService,
    protected searchService: SearchService,
    protected uiconfigsService: UIConfigsService,
    protected monitoringService: MonitoringService,
    protected cdr: ChangeDetectorRef,
    protected controllerService: ControllerService,
    protected iterableDiffers: IterableDiffers
    ) {
      super(controllerService, cdr, uiconfigsService);
      this.arrayDiffers = iterableDiffers.find([]).create(HttpEventUtility.trackBy);
     }


  ngAfterViewInit() {
    // Advanced search panel will only be shown for Alertseventspage HTML and when the user has archive permissions
    if (this.showEventsAdvSearch && this.archivePermissions) {
      setTimeout(() => {
        this.getAdvSearchEvents(this.startingSortField, this.startingSortOrder);
      });
    }
  }

  checkPermissions(): boolean {
    const boolArchiveRequestActions = this.uiconfigsService.isAuthorized(UIRolePermissions['monitoringarchiverequest_all-actions']);
    const boolObjStoreCreate = this.uiconfigsService.isAuthorized(UIRolePermissions.objstoreobject_create);
    return (boolObjStoreCreate && boolArchiveRequestActions);
  }

  postNgInit(): void {
    // Advanced search panel will only be shown for Alertseventspage HTML and when the user has archive permissions
    this.archivePermissions = this.checkPermissions();
    if (this.showEventsAdvSearch && this.archivePermissions) {
      this.populateFieldSelector();
      this.buildAdvSearchCols();
      this.watchArchiveObject();
    } else {
      this.genQueryBodies();
      // Disabling search to reduce scope for august release
      // Adding <any> to prevent typescript compilation from failing due to unreachable code
      if (<any>false) {
        // After user stops typing for 1 second, we invoke a search request to elastic
        const subscription =
          this.eventSearchFormControl.valueChanges.pipe(
            debounceTime(1000),
            distinctUntilChanged()
          ).subscribe(
            value => {
              this.invokeEventsSearch();
            }
          );
        this.subscriptions.push(subscription);
      }

      // If get alerts/events wasn't triggered by on change
      if (!this.eventsSubscription) {
        this.getEvents();
      }

      if (this.searchedEvent) {
        this.getSearchedEvent();
      }
    }
  }

  buildAdvSearchCols() {
    this.advSearchCols = this.cols.filter((col: TableCol) => {
      return (col.field !== 'meta.mod-time' && col.field !== 'source' && col.field !== 'object-ref' );
    });

    this.advSearchCols.push(
      { field: 'object-ref.kind', header: 'Object-Ref Kind', kind: 'Event' },
      { field: 'object-ref.name', header: 'Object-Ref Name', kind: 'Event' },
      { field: 'source.node-name', header: 'Source Node', kind: 'Event' },
      { field: 'source.component', header: 'Source Component', kind: 'Event' }
    );
  }

  /**
   * User will only be allowed to fire only one archive Event request at a time
   * Show the status of the archive request in display panel
   * Allow user to download files on success or cancel when the request is running
   */
  ngDoCheck() {
    if (this.showEventsAdvSearch && this.archivePermissions) {
      const changes = this.arrayDiffers.diff(this.exportedArchiveRequests);
      if (changes) {
        this.handleArchiveLogChange();
      }
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
          // disable the archive request button, show cancel
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

  /**
   * Cancel any Running Archive Request, TODO: Currently Cancel API not ready
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
   * Watch on Archive Event Requests
   * Show any running archive Event request when the user comes on this page
   */
  watchArchiveObject() {
    this.archiveRequestsEventUtility = new HttpEventUtility<MonitoringArchiveRequest>(MonitoringArchiveRequest);
    this.exportedArchiveRequests = this.archiveRequestsEventUtility.array;
    const sub = this.monitoringService.WatchArchiveRequest({ 'field-selector': 'spec.type=event' }).subscribe(
      (response) => {
        this.currentArchReqLength = this.exportedArchiveRequests.length;
        this.archiveRequestsEventUtility.processEvents(response);
        if (this.exportedArchiveRequests.length > 0 && this.exportedArchiveRequests[0].status.status === MonitoringArchiveRequestStatus_status.running) {
          this.enableExport = false;
          this.displayArchPanel = true;
        }
      },
      this.controllerService.webSocketErrorHandler('Failed to get Event Archive Requests')
    );
    this.subscriptions.push(sub);
  }

  getArchiveQuery(archQuer: IMonitoringArchiveQuery) {
    this.eventArchiveQuery = archQuer;
  }

  showArchiveStatusPanel() {
    this.displayArchPanel = true;
  }

  getAdvSearchEvents(field = this.tableContainer.table.sortField, order = this.tableContainer.table.sortOrder) {
    this.eventsLoading = true;
    try {
        const searchSearchRequest = this.advancedSearchComponent.getSearchRequest(field, order, 'Event', false, this.maxRecords);
        this.advancedSearchComponent.emitArchiveQuery();
        this._callSearchRESTAPI(searchSearchRequest);
      } catch (error) {
        this.controllerService.invokeErrorToaster('Input Error', error.toString());
      }
  }

  private _callSearchRESTAPI(searchSearchRequest: SearchSearchRequest) {
    const subscription = this.searchService.PostQuery(searchSearchRequest).subscribe(

      response => {
        const data: SearchSearchResponse = response.body as SearchSearchResponse;
        let objects = data.entries;
        if (!objects || objects.length === 0) {
          this.controllerService.invokeInfoToaster('Information', 'No Events found. Please change search criteria.');
          objects = [];
        }
        const entries = [];
        for (let k = 0; k < objects.length; k++) {
          entries.push(objects[k].object); // objects[k] is a SearchEntry object
        }
        this.persistentEvents = entries;
        this.eventsLoading = false;
        this.eventsTimeBased = this.combineEvents();
        this.getEvents();
      },
      (error) => {
        this.eventsLoading = false;
        this.controllerService.invokeRESTErrorToaster('Failed to get events', error);
      }
    );
    this.subscriptions.push(subscription);
  }

  /**
   * This serves HTML template
   * @param $event
   */
  handleFieldRepeaterData(values) {
    this.eventFieldSelectorOutput = values;
  }

  populateFieldSelector() {
    this.fieldFormArray = new FormArray([]);
  }

  /**
   * This serves HTML API. It clear audit-event search and refresh data.
   * @param $event
   */
  onCancelEventSearch($event) {
    this.currentSearchCriteria = '';
    this.currentEventSeverityFilter = null;
    this.populateFieldSelector();
    this.getAdvSearchEvents();
    this.controllerService.invokeInfoToaster('Infomation', 'Cleared search criteria, events refreshed.');
  }

  getSearchedEvent() {
    this.eventsService.GetGetEvent(this.searchedEvent).subscribe(
      (response) => {
        this.selectedEvent = response.body as EventsEvent;
      },
      (error) => {
        // User may tweak browser url and make invalid event name in the url, we will catch and throw error.
        this.selectedEvent = null;
        this.controllerService.invokeRESTErrorToaster('Failed to fetch event ' + this.searchedEvent, error);
      }
    );
  }

  closeDetails() {
    this.selectedEvent = null;
  }

  countOnValidation(data) {
    if (data == null) {
      return;
    }
    // Check that there is new data
    if (this.events.length === data.length) {
      // Both sets of data are empty
      if (this.events.length === 0) {
        return;
      }
    }
    this.events = data;
    // Reset counters
    Object.keys(EventsEvent_severity).forEach(severity => {
      this.eventNumbers[severity] = 0;
    });
    data.forEach(event => {
      this.eventNumbers[event.severity] += 1;
    });
    this.filterEvents();
  }

  getEvents() {
    if (!this.showEventsAdvSearch || !this.archivePermissions) {
      if (this.eventsSubscription) {
        this.eventsSubscription.unsubscribe();
        this.eventsService.pollingUtility.terminatePolling(this.pollingServiceKey, true);
      }
      this.eventsSubscription = this.eventsService.pollEvents(this.pollingServiceKey, this.eventsPostBody).subscribe(
        (data) => {
          this.countOnValidation(data);
        }
      );
    } else {
      this.countOnValidation(this.eventsTimeBased);
    }
  }

  /**
   * This API serves html template.
   * It will filter events displayed in table
   */
  onEventNumberClick(event, severityType: string) {
    if (this.currentEventSeverityFilter === severityType) {
      this.currentEventSeverityFilter = null;
    } else {
      this.currentEventSeverityFilter = severityType;
    }
    this.filterEvents();
    // Disabling search to reduce scope for august release
    // Adding <any> to prevent typescript compilation from failing due to unreachable code
    if (<any>false) {
      // TODO: Add support for searching for events through elastic
      this.invokeEventsSearch();
    }
  }

  showDebugPressed() {
    if (!this.showEventsAdvSearch || !this.archivePermissions) {
      this.genQueryBodies();
    }
    this.getEvents();
  }

  /**
   * Combines results based on time and advanced search
   */
  combineEvents(): EventsEvent[] {
    const eventsMeta: string[] = [];
    const resultObjects: EventsEvent[] = [];

    this.persistentEvents.forEach(eve => eventsMeta.push(eve.meta.name));
    this.eventsTimeBased.forEach(res => {
      if (eventsMeta.includes(res.meta.name)) {
        resultObjects.push(res);
      }
    });
    return resultObjects;
  }

  filterEvents() {
    this.eventsLoading = true;
    // We put the filtering into a set timeoute so that it gets pushed to the end of
    // the micro task queue.
    // Otherwise, the table rendering of the items happens before the user's action on the checkbox
    // becomes visible. This allows the checkbox animation to happen immediately, and then we render
    // the new table.
    setTimeout(() => {
      // checking whether to show debug events
      if (this.showDebugEvents) {
        this.dataObjects = this.events;
      } else {
        this.dataObjects = this.events.filter(item => item.severity !== EventsEvent_severity.debug);
      }
      this.eventsTotalCount = this.dataObjects.length;
      if (this.currentEventSeverityFilter != null) {
        this.dataObjects = this.dataObjects.filter(item => item.severity === this.currentEventSeverityFilter);
      }

      this.eventsLoading = false;
    }, 0);
  }

  /**
   * Makes a request to elastic using the current filters
   * If there are no filters, the function sets filteredEvents
   * to be all events and returns.
   *
   * Unused for now to reduce scope for August release
   */
  invokeEventsSearch() {
    // If no text or severity filters, we set to all events
    // and skip making a query to elastic
    if (this.currentEventSeverityFilter == null &&
      (this.eventSearchFormControl.value == null ||
        this.eventSearchFormControl.value.trim().length === 0)) {
      this.dataObjects = this.events;
      return;
    }

    this.eventsLoading = true;
    const body = new SearchSearchRequest();
    body['max-results'] = 100;
    body.query.kinds = ['Event'];
    body['sort-by'] = 'meta.mod-time';

    if (this.currentEventSeverityFilter != null) {
      const requirement = new FieldsRequirement();
      requirement.key = 'severity';
      requirement.operator = FieldsRequirement_operator.equals;
      requirement.values = [this.currentEventSeverityFilter];
      body.query.fields.requirements = [requirement];
    }
    // Don't add search requirement if its empty space or not set
    if (this.eventSearchFormControl.value != null
      && this.eventSearchFormControl.value.trim().length !== 0) {
      const text = new SearchTextRequirement();
      const input = this.eventSearchFormControl.value.trim();
      if (input.split(' ').length > 1) {
        text.text = input.split(' ');
      } else {
        // adding wild card matching to the end as user
        // may not be done with search
        text.text = [input + '*'];
      }
      body.query.texts = [text];
    }

    this.searchService.PostQuery(body).subscribe(
      (data) => {
        const respBody = data.body as ISearchSearchResponse;
        this.dataObjects = [];
        if (respBody.entries != null) {
          // We reverse the entries to get the sorted order to be latest time first
          respBody.entries.reverse().forEach(entry => {
            const event = entry.object as IEventsEvent;
            const match = this.eventMap[event.meta.name];
            if (match != null) {
              this.dataObjects.push(match);
            }
          });
        }
        this.eventsLoading = false;
      }
    );
  }

  setEventsTimeRange(timeRange: TimeRange) {
    // update query and call getEvents
    setTimeout(() => {
      this.eventsSelectedTimeRange = timeRange;
      const start = this.eventsSelectedTimeRange.getTime().startTime.toISOString() as any;
      const end = this.eventsSelectedTimeRange.getTime().endTime.toISOString() as any;
      this.eventsTimeConstraints = 'meta.creation-time<' + end + ',' + 'meta.creation-time>' + start;
      if (this.showEventsAdvSearch && this.archivePermissions) {
        this.genTimeBasedSearch();
      } else {
        this.genQueryBodies();
        this.getEvents();
      }
    }, 0);
  }

  genTimeBasedSearch() {
    this.eventsSubscription = this.eventsService.PostGetEvents({'field-selector': this.eventsTimeConstraints}).subscribe(
      (response) => {
        const data: EventsEventList = response.body as EventsEventList;
        this.eventsTimeBased = [];
        if (data.items !== undefined && data.items !== null) {
          this.eventsTimeBased = data.items;
        }
        // When page is loaded, this function is called before advanced search. At that time calling combineEvents doesn't make sense
        this.eventsTimeBased = this.persistentEvents.length !== 0 ? this.combineEvents() : this.eventsTimeBased;
        this.getEvents();
      },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Failed to get events', error);
      }
    );
  }

  genQueryBodies() {

    const fieldSelectorOptions = [];

    if (this.selector != null) {
     fieldSelectorOptions.push(this.selector.eventSelector.selector);
    }
    if (this.eventsTimeConstraints.length) {
      fieldSelectorOptions.push(this.eventsTimeConstraints);
    }
    if (!this.showDebugEvents) {
      fieldSelectorOptions.push('severity!=debug');
    }

    this.eventsPostBody = {
      'field-selector': fieldSelectorOptions.join(','),
      'sort-order': ApiListWatchOptions_sort_order.none
    };
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
  }

  ngOnChanges(change: SimpleChanges) {
    if (!this.showEventsAdvSearch || !this.archivePermissions) {
      if (change.selector) {
        this.genQueryBodies();
        this.getEvents();
      }
      if (change.searchedEvent && this.searchedEvent) {
          this.getSearchedEvent();
      }
    }
  }

  ngOnDestroy() {
    this.subscriptions.forEach(
      subscription => {
        subscription.unsubscribe();
      }
    );

    if (this.eventsSubscription != null) {
      this.eventsSubscription.unsubscribe();
    }
  }

  selectEvent($event) {
    if ( this.selectedEvent && $event.rowData.meta.name === this.selectedEvent.meta.name ) {
      this.selectedEvent = null;
    } else {
      this.selectedEvent = $event.rowData;
    }
  }

  dateToString(date) {
    const prettyDate = new PrettyDatePipe('en-US');
    return prettyDate.transform(date);
  }

  setDefaultToolbar(): void {
  }

  deleteRecord(object: EventsEvent): Observable<{ body: IEventsEvent | IApiStatus | Error; statusCode: number; }> {
    throw new Error('Method not supported.');
  }

  generateDeleteConfirmMsg(object: EventsEvent): string {
    throw new Error('Method not supported.');
  }

  generateDeleteSuccessMsg(object: EventsEvent): string {
    throw new Error('Method not supported.');
  }
}
