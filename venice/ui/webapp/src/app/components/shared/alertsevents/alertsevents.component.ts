import { Component, Input, OnDestroy, OnInit, ViewChild, ViewEncapsulation, SimpleChanges, OnChanges, Output, EventEmitter } from '@angular/core';
import { FormControl } from '@angular/forms';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { AlerttableService } from '@app/services/alerttable.service';
import { ControllerService } from '@app/services/controller.service';
import { EventsService } from '@app/services/events.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { SearchService } from '@app/services/generated/search.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { EventsEvent_severity, EventsEventAttributes_severity, IApiListWatchOptions, IEventsEvent, ApiListWatchOptions_sort_order } from '@sdk/v1/models/generated/events';
import { MonitoringAlert, MonitoringAlertSpec_state, MonitoringAlertStatus_severity, MonitoringAlertSpec_state_uihint } from '@sdk/v1/models/generated/monitoring';
import { FieldsRequirement, FieldsRequirement_operator, ISearchSearchResponse, SearchSearchQuery_kinds, SearchSearchRequest, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import { Table } from 'primeng/table';

import { Observable, forkJoin, throwError, Subscription } from 'rxjs';
import { debounceTime, distinctUntilChanged, switchMap } from 'rxjs/operators';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

export interface AlertsEventsSelector {
  alertSelector: {
    selector: string,
    name: string
  };
  eventSelector: {
    selector: string,
    name: string
  };
}

/**
 * Renders two tabs that displays an alerts table and an events table.
 *
 * Alerts table is currently using mocked data.
 *
 * Events are poll the list endpoint without any filters
 * To apply filters, we make a request to elastic and then map the returned meta
 * to objects we already have.
 * After making an elastic search, when new events come in we redo the search to elastic
 *
 */
@Component({
  selector: 'app-shared-alertsevents',
  templateUrl: './alertsevents.component.html',
  styleUrls: ['./alertsevents.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertseventsComponent extends BaseComponent implements OnInit, OnDestroy, OnChanges {
  @ViewChild('alerttable') alertTurboTable: Table;
  @ViewChild('eventTable') eventTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;

  // Used as the key for uniquely identifying poll requests.
  // If there are going to be two of these components alive at the same time,
  // this field is required for them to have independent queries.
  @Input() pollingServiceKey: string = 'alertsevents';
  // If provided, will only show alerts and events
  // where the source node matches
  @Input() selector: AlertsEventsSelector;
  @Output() activeTab: EventEmitter<string> = new EventEmitter<string>();

  // this property indicate if user is authorized to update alerts
  alertUpdatable: boolean = true;

  subscriptions: Subscription[] = [];
  severityEnum = EventsEventAttributes_severity;

  alertSubscription: Subscription;
  eventsSubscription: Subscription;

  // EVENTS
  // Used for the table - when true there is a loading icon displayed
  eventsLoading = false;

  // Holds all events
  events: IEventsEvent[] = [];
  // Contains the total number of events.
  // Count does not include debug events if show debug events is not selected
  eventsTotalCount = 0;

  // Mapping from meta.name to event object. Used for mapping entries from elastic
  // to the event objects we have.
  eventMap = {};

  // holds a subset (possibly all) of this.events
  // This are the events that will be displayed
  filteredEvents: IEventsEvent[] = [];

  eventsPostBody: IApiListWatchOptions = { 'sort-order': ApiListWatchOptions_sort_order.None };

  // All columns are set as not sortable as it isn't currently supported
  // TODO: Support sorting columns
  eventCols: any[] = [
    { field: 'severity', header: 'Severity', class: 'alertsevents-column-severity', sortable: false },
    { field: 'type', header: 'Type', class: 'alertsevents-column-event', sortable: true },
    { field: 'message', header: 'Message', class: 'alertsevents-column-message-event', sortable: false },
    { field: 'object-ref', header: 'Object Ref', class: 'alertsevents-column-obj-ref', sortable: false },
    { field: 'count', header: 'Count', class: 'alertsevents-column-count', sortable: false },
    { field: 'source', header: 'Source Node & Component', class: 'alertsevents-column-source', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'alertsevents-column-date', sortable: true },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'alertsevents-column-date', sortable: true },
  ];

  // Will hold mapping from severity types to counts
  eventNumbers: { [severity in EventsEvent_severity]: number } = {
    'INFO': 0,
    'WARN': 0,
    'CRITICAL': 0,
    'DEBUG': 0
  };

  eventsIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'event'
  };

  alertsIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'notifications'
  };

  eventSearchFormControl: FormControl = new FormControl();

  // The current severity filter, set to null if it is on All.
  currentEventSeverityFilter = null;

  // ALERTS
  // Used for the table - when true there is a loading icon displayed
  alertsLoading = false;

  // Used for processing watch stream events
  alertsEventUtility: HttpEventUtility<MonitoringAlert>;

  // Holds all alerts
  alerts: ReadonlyArray<MonitoringAlert> = [];

  // holds a subset (possibly all) of this.alerts
  // This are the alerts that will be displayed
  filteredAlerts: ReadonlyArray<MonitoringAlert> = [];
  alertCols: any[] = [
    { field: 'meta.mod-time', header: 'Modification Time', class: 'alertsevents-column-date', sortable: true },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'alertsevents-column-date', sortable: true },
    { field: 'status.severity', header: 'Severity', class: 'alertsevents-column-severity', sortable: false },
    { field: 'status.message', header: 'Message', class: 'alertsevents-column-message', sortable: false },
    { field: 'spec.state', header: 'State', class: 'alertsevents-column-state', sortable: false },
    { field: 'status.source', header: 'Source Node & Component', class: 'alerts-column-source', sortable: false, isLast: true },
  ];

  // Selected alerts
  selectedAlerts: MonitoringAlert[];

  // Will hold mapping from severity types to counts
  alertNumbers: { [severity in MonitoringAlertStatus_severity]: number } = {
    'INFO': 0,
    'WARN': 0,
    'CRITICAL': 0,
  };

  // The current alert severity filter, set to null if it is on All.
  currentAlertSeverityFilter;

  // Alert State filters
  selectedStateFilters = [MonitoringAlertSpec_state_uihint.OPEN];
  possibleFilterStates = Object.values(MonitoringAlertSpec_state_uihint);

  showDebugEvents: boolean = false;

  alertTrackBy = HttpEventUtility.trackBy;

  // Query params to send for watch
  alertQuery = {};

  constructor(protected _controllerService: ControllerService,
    protected _alerttableService: AlerttableService,
    protected uiconfigsService: UIConfigsService,
    protected searchService: SearchService,
    protected eventsService: EventsService,
    protected monitoringService: MonitoringService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {

    this.alertUpdatable = this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringalert_update);
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
    if (!this.alertSubscription || !this.eventsSubscription) {
      this.getObjects();
    }
  }

  getObjects() {
    this.getAlerts();
    this.getEvents();
  }

  ngOnChanges(change: SimpleChanges) {
    if (change.selector) {
      this.genQueryBodies();
      this.getObjects();
    }
  }

  emitActiveTab(tabIndex) {
    if (tabIndex === 0) {
      this.activeTab.emit('alerts');
    } else {
      this.activeTab.emit('events');
    }
  }

  genQueryBodies() {
    if (this.selector != null) {
      this.eventsPostBody = {
        'field-selector': this.selector.eventSelector.selector,
        'sort-order': ApiListWatchOptions_sort_order.None
      };
      this.alertQuery = {
        'field-selector': this.selector.alertSelector.selector
      };
    }
  }

  getClassName(): string {
    return this.constructor.name;
  }

  resizeTable(delay: number = 0) {
    if (this.lazyRenderWrapper) {
      this.lazyRenderWrapper.resizeTable(delay);
    }
  }

  selectedIndexChangeEvent(event) {
    this.emitActiveTab(event);
    if (this.lazyRenderWrapper != null) {
      this.lazyRenderWrapper.resizeTable();
    }
  }


  getEvents() {
    if (this.eventsSubscription) {
      this.eventsSubscription.unsubscribe();
    }
    this.eventsSubscription = this.eventsService.pollEvents(this.pollingServiceKey, this.eventsPostBody).subscribe(
      (data) => {
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
    );
  }

  getAlerts() {
    this.filteredAlerts = [];
    this.alertsEventUtility = new HttpEventUtility<MonitoringAlert>(MonitoringAlert);
    this.alerts = this.alertsEventUtility.array;
    if (this.alertSubscription) {
      this.alertSubscription.unsubscribe();
    }
    this.alertSubscription = this.monitoringService.WatchAlert(this.alertQuery).subscribe(
      response => {
        this.alertsEventUtility.processEvents(response);
        // Reset counters
        Object.keys(MonitoringAlertStatus_severity).forEach(severity => {
          this.alertNumbers[severity] = 0;
        });
        this.alerts.forEach(alert => {
          this.alertNumbers[alert.status.severity] += 1;
        });
        this.filterAlerts();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Alert')
    );
    this.subscriptions.push(this.alertSubscription);
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
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

  /**
   * This API serves html template.
   * It will filter events displayed in table
   */
  onAlertNumberClick(severityType: string) {
    if (this.currentAlertSeverityFilter === severityType) {
      this.currentAlertSeverityFilter = null;
    } else {
      this.currentAlertSeverityFilter = severityType;
    }
    this.filterAlerts();
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
        this.filteredEvents = this.events;
      } else {
        this.filteredEvents = this.events.filter(item => item.severity !== EventsEvent_severity.DEBUG);
      }
      this.eventsTotalCount = this.filteredEvents.length;

      if (this.currentEventSeverityFilter != null) {
        this.filteredEvents = this.filteredEvents.filter(item => item.severity === this.currentEventSeverityFilter);
      }

      this.eventsLoading = false;
      this.lazyRenderWrapper.resetTableView();
    }, 0);
  }

  filterAlerts() {
    // We put the filtering into a set timeout so that it gets pushed to the end of
    // the micro task queue.
    // Otherwise, the table rendering of the items happens before the user's action on the checkbox
    // becomes visible. This allows the checkbox animation to happen immediately, and then we render
    // the new table.
    setTimeout(() => {
      this.filteredAlerts = this.alerts.filter((item) => {
        // Checking severity filter
        if (this.currentAlertSeverityFilter != null && item.status.severity !== this.currentAlertSeverityFilter) {
          return false;
        }
        // checking state filter
        return this.selectedStateFilters.includes(MonitoringAlertSpec_state_uihint[item.spec.state]);
      });
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
      this.filteredEvents = this.events;
      return;
    }

    this.eventsLoading = true;
    const body = new SearchSearchRequest();
    body['max-results'] = 100;
    body.query.kinds = [SearchSearchQuery_kinds.Event];
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
        this.filteredEvents = [];
        if (respBody.entries != null) {
          // We reverse the entries to get the sorted order to be latest time first
          respBody.entries.reverse().forEach(entry => {
            const event = entry.object as IEventsEvent;
            const match = this.eventMap[event.meta.name];
            if (match != null) {
              this.filteredEvents.push(match);
            }
          });
        }
        this.eventsLoading = false;
      }
    );
  }

  /**
   * Submits an HTTP request to mark the alert as resolved
   * @param alert Alert to resolve
   */
  resolveAlert(alert: MonitoringAlert) {
    const summary = 'Alert Resolved';
    const msg = 'Marked alert as resolved';
    this.updateAlertState(alert, MonitoringAlertSpec_state.RESOLVED, summary, msg);
  }

  acknowledgeAlert(alert) {
    const summary = 'Alert Acknowledged';
    const msg = 'Marked alert as acknowledged';
    this.updateAlertState(alert, MonitoringAlertSpec_state.ACKNOWLEDGED, summary, msg);
  }

  openAlert(alert) {
    const summary = 'Alert Opened';
    const msg = 'Marked alert as open';
    this.updateAlertState(alert, MonitoringAlertSpec_state.OPEN, summary, msg);
  }

  /**
   * Submits an HTTP request to update the state of the alert
   * @param alert Alert to resolve
   */
  updateAlertState(alert: MonitoringAlert, newState: MonitoringAlertSpec_state, summary: string, msg: string) {
    // Create copy so that when we modify it doesn't change the view
    const observable = this.buildUpdateAlertStateObservable(alert, newState);
    const subscription = observable.subscribe(
      response => {
        this._controllerService.invokeSuccessToaster(summary, msg);
      },
      this._controllerService.restErrorHandler(summary + ' Failed')
    );
    this.subscriptions.push(subscription);
  }

  buildUpdateAlertStateObservable(alert: MonitoringAlert, newState: MonitoringAlertSpec_state): Observable<any> {
    const payload = new MonitoringAlert(alert);
    payload.spec.state = newState;
    const observable = this.monitoringService.UpdateAlert(payload.meta.name, payload);
    return observable;
  }

  ngOnDestroy() {
    this.subscriptions.forEach(
      subscription => {
        subscription.unsubscribe();
      }
    );

    if (this.alertSubscription != null) {
      this.alertSubscription.unsubscribe();
    }
    if (this.eventsSubscription != null) {
      this.eventsSubscription.unsubscribe();
    }

  }

  /**
   * This api serves html template
   */
  resolveSelectedAlerts() {
    const summary = 'Alerts Resolved';
    const msg = 'Marked selected alerts as resolved';
    const newState = MonitoringAlertSpec_state.RESOLVED;
    const observables = this.buildObservaleList(newState);
    this.updateAlertList(observables, summary, msg);
  }

  /**
   * This api serves html template
   */
  acknowledgeSelectedAlerts() {
    const summary = 'Alerts Acknowledged';
    const msg = 'Marked selected alerts as acknowledged';
    const newState = MonitoringAlertSpec_state.ACKNOWLEDGED;
    const observables = this.buildObservaleList(newState);
    this.updateAlertList(observables, summary, msg);
  }

  /**
   * This api serves html template
   */
  openSelectedAlerts() {
    const summary = 'Alerts Opened';
    const msg = 'Marked selected alerts as open';
    const newState = MonitoringAlertSpec_state.OPEN;
    const observables = this.buildObservaleList(newState);
    this.updateAlertList(observables, summary, msg);
  }

  buildObservaleList(newState: MonitoringAlertSpec_state): Observable<any>[] {
    const observables = [];
    for (let i = 0; this.selectedAlerts && i < this.selectedAlerts.length; i++) {
      const observable = this.buildUpdateAlertStateObservable(this.selectedAlerts[i], newState);
      observables.push(observable);
    }
    return observables;
  }

  updateAlertList(observables: Observable<any>[], summary: string, msg: string) {
    if (observables.length <= 0) {
      return;
    }
    forkJoin(observables).subscribe((results) => {
      const isAllOK = Utility.isForkjoinResultAllOK(results);
      if (isAllOK) {
        this._controllerService.invokeSuccessToaster(summary, msg);
        this.selectedAlerts = []; // clear this.selectedAlerts
      } else {
        const error = Utility.joinErrors(results);
        this._controllerService.invokeRESTErrorToaster(summary, error);
      }
    });
  }

  /**
   * This is a helper function
   * @param state
   * @param reversed
   * this.showBatchIconHelper(MonitoringAlertSpec_state.RESOLVED, true);
   *      means that we want selected alerts all NOT in RESOLVED state
   * this.showBatchIconHelper(MonitoringAlertSpec_state.OPEN, false);
   *      means that we want selected alerts all in OPEN state
   */
  showBatchIconHelper(state: MonitoringAlertSpec_state, reversed: boolean = true): boolean {
    if (!this.alertUpdatable || !this.selectedAlerts || this.selectedAlerts.length === 0) {
      return false;
    }
    for (let i = 0; i < this.selectedAlerts.length; i++) {
      const alert = this.selectedAlerts[i];
      if (!reversed) {
        if (alert.spec.state !== state) {
          return false;
        }
      } else {
        if (alert.spec.state === state) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * This api serves html template
   */
  showBatchResolveIcon(): boolean {
    // we want selected alerts all NOT in RESOLVED state
    return this.showBatchIconHelper(MonitoringAlertSpec_state.RESOLVED, true);
  }

  /**
   * This api serves html template
   */
  showBatchAcknowLedgeIcon(): boolean {
    // we want selected alerts all NOT in ACKNOWLEDGED state
    return this.showBatchIconHelper(MonitoringAlertSpec_state.ACKNOWLEDGED, true);
  }

  /**
   * This api serves html template
   */
  showBatchOpenIcon(): boolean {
    // we want selected alerts all NOT in OPEN state
    return this.showBatchIconHelper(MonitoringAlertSpec_state.OPEN, true);
  }
}
