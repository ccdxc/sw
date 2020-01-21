import { Component, OnInit, Input, ChangeDetectorRef, SimpleChanges, OnChanges, OnDestroy, ViewEncapsulation } from '@angular/core';
import { Observable, forkJoin, throwError, Subscription } from 'rxjs';
import { EventsEvent_severity, EventsEventAttributes_severity, IApiListWatchOptions, IEventsEvent, EventsEvent, ApiListWatchOptions_sort_order } from '@sdk/v1/models/generated/events';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { EventsService } from '@app/services/events.service';
import { FormControl } from '@angular/forms';
import { FieldsRequirement, FieldsRequirement_operator, ISearchSearchResponse, SearchSearchRequest, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import { SearchService } from '@app/services/generated/search.service';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ControllerService } from '@app/services/controller.service';
import { debounceTime, distinctUntilChanged, switchMap } from 'rxjs/operators';
import { AlertsEventsSelector } from '@app/components/shared/alertsevents/alertsevents.component';
import { IApiStatus } from '@sdk/v1/models/generated/search';
import { Animations } from '@app/animations';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';

@Component({
  selector: 'app-eventstable',
  templateUrl: './eventstable.component.html',
  styleUrls: ['./eventstable.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventstableComponent extends TablevieweditAbstract<IEventsEvent, EventsEvent> implements OnChanges, OnDestroy {

  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = false;
  exportFilename: string;

  // Used as the key for uniquely identifying poll requests.
  // If there are going to be two of these components alive at the same time,
  // this field is required for them to have independent queries.
  @Input() pollingServiceKey: string = 'alertsevents';
  // If provided, will only show alerts and events
  // where the source node matches
  @Input() selector: AlertsEventsSelector;
  @Input() searchedEvent: string;

  currentEventSeverityFilter = null;
  showDebugEvents: boolean = false;
  eventsSubscription: Subscription;
  eventSearchFormControl: FormControl = new FormControl();
  severityEnum = EventsEventAttributes_severity;

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

  eventsPostBody: IApiListWatchOptions = { 'sort-order': ApiListWatchOptions_sort_order.none };

  // All columns are set as not sortable as it isn't currently supported
  // TODO: Support sorting columns
  cols: TableCol[] = [
    { field: 'severity', header: 'Severity', class: '', sortable: false, width: 10 },
    { field: 'type', header: 'Type', class: '', sortable: true, width: 8 },
    { field: 'message', header: 'Message', class: '', sortable: false, width: 30 },
    { field: 'object-ref', header: 'Object Ref', class: '', sortable: false, width: 10 },
    { field: 'count', header: 'Count', class: '', sortable: false, width: 5 },
    { field: 'source', header: 'Source Node & Component', class: '', sortable: false, width: 11 },
    { field: 'meta.mod-time', header: 'Modification Time', class: '', sortable: true, width: 13 },
    { field: 'meta.creation-time', header: 'Creation Time', class: '', sortable: true, width: 13 },
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

  constructor(protected eventsService: EventsService,
    protected searchService: SearchService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected controllerService: ControllerService
    ) {
      super(controllerService, cdr, uiconfigsService);
     }

  postNgInit(): void {
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

  getEvents() {
    if (this.eventsSubscription) {
      this.eventsSubscription.unsubscribe();
      this.eventsService.pollingUtility.terminatePolling(this.pollingServiceKey, true);
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
      this.genQueryBodies();
      this.getEvents();
    }, 0);
  }

  genQueryBodies() {

    const fieldSelectorOptions = [];

    if (this.selector != null) {
     fieldSelectorOptions.push(this.selector.eventSelector.selector);
    }
    if (this.eventsTimeConstraints.length) {
      fieldSelectorOptions.push(this.eventsTimeConstraints);
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
    if (change.selector) {
      this.genQueryBodies();
      this.getEvents();
    }
    if (change.searchedEvent && this.searchedEvent) {
        this.getSearchedEvent();
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
    throw new Error('Method not implemented.');
  }
  generateDeleteConfirmMsg(object: EventsEvent): string {
    throw new Error('Method not implemented.');
  }
  generateDeleteSuccessMsg(object: EventsEvent): string {
    throw new Error('Method not implemented.');
  }
}
