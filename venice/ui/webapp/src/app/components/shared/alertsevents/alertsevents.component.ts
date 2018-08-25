import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation, Input } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { AlerttableService } from '@app/services/alerttable.service';
import { ControllerService } from '@app/services/controller.service';
import { EventsService } from '@app/services/events.service';
import { EventsEvent_severity, EventsEvent_severity_uihint, IEventsEvent, IApiListWatchOptions } from '@sdk/v1/models/generated/events';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs/Subscription';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SearchService } from '@app/services/generated/search.service';
import { SearchSearchQuery, SearchSearchRequest, SearchSearchQuery_kinds, FieldsRequirement, FieldsRequirement_operator, ISearchSearchResponse, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { FormControl } from '@angular/forms';
import 'rxjs/add/operator/debounceTime';
import 'rxjs/add/operator/distinctUntilChanged';

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
export class AlertseventsComponent extends BaseComponent implements OnInit, OnDestroy {
  // Feature Flags
  hideAlerts: boolean = this.uiconfigsService.isObjectDisabled('Alerts');

  @ViewChild('alerttable') alertTurboTable: Table;
  @ViewChild('eventTable') eventTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;

  // Used as the key for uniquely identifying poll requests.
  // If there are going to be two of these components alive at the same time,
  // this field is required for them to have independent queries.
  @Input() pollingServiceKey: string = 'alertsevents';

  subscriptions: Subscription[] = [];
  severityEnum = EventsEvent_severity_uihint;

  // EVENTS
  // Used for the table - when true there is a loading icon displayed
  eventsLoading = false;

  // Holds all events
  events: IEventsEvent[] = [];

  // Mapping from meta.name to event object. Used for mapping entries from elastic
  // to the event objects we have.
  eventMap = {};

  // holds a subset (possibly all) of this.events
  // This are the events that will be displayed
  filteredEvents: IEventsEvent[] = [];

  eventsPostBody: IApiListWatchOptions = {};

  // All columns are set as not sortable as it isn't currently supported
  // TODO: Support sorting columns
  eventCols: any[] = [
    { field: 'meta.mod-time', header: 'Modification Time', class: 'alertsevents-column-date', sortable: false },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'alertsevents-column-date', sortable: false },
    { field: 'severity', header: 'Severity', class: 'alertsevents-column-severity', sortable: false },
    { field: 'type', header: 'Type', class: 'alertsevents-column-event', sortable: false },
    { field: 'message', header: 'Message', class: 'alertsevents-column-message', sortable: false },
    { field: 'source', header: 'Source Node & Component', class: 'alertsevents-column-source', sortable: false },
    { field: 'count', header: 'Count', class: 'alertsevents-column-count', sortable: false },
  ];

  // Will hold mapping from severity types to counts
  protected eventNumbers = {
  };

  eventsIcon: Icon = {
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
  alertCols: any[] = [
    { field: 'date', header: 'Date', class: 'alertsevents-column-date', sortable: true },
    { field: 'name', header: 'Name', class: 'alertsevents-column-name', sortable: false },
    { field: 'severity', header: 'Severity', class: 'alertsevents-column-severity', sortable: false },
    { field: 'message', header: 'Message', class: 'alertsevents-column-message', sortable: false },
    { field: 'policy', header: 'Policy', class: 'alertsevents-column-policy', sortable: false },
  ];
  alerts: any;
  selectedAlerts: any = [];


  protected alertnumber = {
    total: 0,
    critical: 0,
    warning: 0,
    info: 0
  };

  constructor(protected _controllerService: ControllerService,
    protected _alerttableService: AlerttableService,
    protected uiconfigsService: UIConfigsService,
    protected searchService: SearchService,
    protected eventsService: EventsService) {
    super(_controllerService);
  }

  ngOnInit() {
    // Disabling search to reduce scope for august release
    // Adding <any> to prevent typescript compilation from failing due to unreachable code
    if (<any>false) {
      // After user stops typing for 1 second, we invoke a search request to elastic
      const subscription =
        this.eventSearchFormControl.valueChanges
          .debounceTime(1000)
          .distinctUntilChanged().subscribe(
            value => {
              this.invokeEventsSearch();
            }
          );
      this.subscriptions.push(subscription);
    }

    this.getAlerts();
    this.getEvents();
  }

  getClassName(): string {
    return this.constructor.name;
  }

  resizeTable(delay: number = 0) {
    if (this.lazyRenderWrapper) {
      this.lazyRenderWrapper.resizeTable(delay);
    }
  }

  getEvents() {
    const subscription = this.eventsService.pollEvents(this.pollingServiceKey, this.eventsPostBody).subscribe(
      (data) => {
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
        if (this.currentEventSeverityFilter == null) {
          this.filteredEvents = data;
        } else {
          this.filteredEvents =
            data.filter(item => item.severity === this.currentEventSeverityFilter);
        }
      }
    );
    this.subscriptions.push(subscription);
  }

  getAlerts() {
    const payload = '';
    this._alerttableService.getAlerts(payload).subscribe(
      data => {
        this.alerts = data;
        this.alertnumber = Utility.computeAlertNumbers(this.alerts);
      },
      err => {
        this.successMessage = '';
        this.errorMessage = 'Failed to get items! ' + err;
        this.error(err);
      }
    );
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
   * This api serves html template
   */
  getAlertItemIconClass(record: IEventsEvent) {
    return 'global-alert-' + record.severity.toLowerCase();
  }

  /**
   * This api serves html template
   */
  onAlerttableArchiveRecord($event, alert) {
    console.log('AlerttableComponent.onAlerttableArchiveRecord()', alert);
  }

  /**
   * This api serves html template
   */
  onAlerttableDeleteRecord($event, alert) {
    console.log('AlerttableComponent.onAlerttableDeleteRecord()', alert);
  }

  /**
   * This API serves html template.
   * It will filter events displayed in table
   */
  onEventNumberClick(event, severityType: string) {
    if (severityType === 'total') {
      this.currentEventSeverityFilter = null;
      // remove severity filter if it exists
    } else {
      this.currentEventSeverityFilter = severityType;
      this.filteredEvents =
        this.events.filter(item => item.severity === this.currentEventSeverityFilter);
    }
    // Disabling search to reduce scope for august release
    // Adding <any> to prevent typescript compilation from failing due to unreachable code
    if (<any>false) {
      // TODO: Add support for searching for events through elastic
      this.invokeEventsSearch();
    }
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
   * This API serves html template.
   * It will filter alerts displayed in list
   */
  onAlertNumberClick(event, alertType: string) {
    if (alertType === 'total') {
      this.alertTurboTable.filter('', 'severity', 'equals');
    } else {
      this.alertTurboTable.filter(alertType, 'severity', 'equals');
    }
  }

  /**
   * This API serves html template.
   * It will delete selected alerts
   */
  onAlerttableDeleteMultiRecords($event) {
    console.log('AlerttableComponent.onAlerttableDeleteMultiRecords()', this.selectedAlerts);
  }

  /**
  * This API serves html template.
  *  It will archive selected alerts
  */
  onAlerttableArchiveMultiRecords($event) {
    console.log('AlerttableComponent.onAlerttableArchiveMultiRecords()', this.selectedAlerts);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(
      subscription => {
        subscription.unsubscribe();
      }
    );
  }

}
