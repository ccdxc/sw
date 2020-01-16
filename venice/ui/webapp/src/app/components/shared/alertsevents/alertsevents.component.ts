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
import { FieldsRequirement, FieldsRequirement_operator, ISearchSearchResponse, SearchSearchRequest, SearchTextRequirement } from '@sdk/v1/models/generated/search';
import { Table } from 'primeng/table';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { TableCol } from '@app/components/shared/tableviewedit';
import { Observable, forkJoin, throwError, Subscription } from 'rxjs';
import { debounceTime, distinctUntilChanged, switchMap } from 'rxjs/operators';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TablevieweditHTMLComponent } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { ActivatedRoute } from '@angular/router';


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
export class AlertseventsComponent extends BaseComponent implements OnInit {

  // Used as the key for uniquely identifying poll requests.
  // If there are going to be two of these components alive at the same time,
  // this field is required for them to have independent queries.
  @Input() pollingServiceKey: string = 'alertsevents';
  // If provided, will only show alerts and events
  // where the source node matches
  @Input() selector: AlertsEventsSelector;
  @Output() activeTab: EventEmitter<string> = new EventEmitter<string>();

  tabIndex: number = 0;
  searchedAlert: string = '';
  searchedEvent: string = '';

  constructor(protected _controllerService: ControllerService,
    private _route: ActivatedRoute
  ) {
    super(_controllerService);
  }

  ngOnInit() {

    this._route.queryParams.subscribe(params => {
      if (params.hasOwnProperty('alert')) {
        // alerttab selected
        this.searchedAlert = params['alert'];
        this.tabIndex = 0;
      }
      if (params.hasOwnProperty('event')) {
        // eventtab selected
        this.tabIndex = 1;
        this.searchedEvent = params['event'];
      }
    });
  }

  emitActiveTab(tabIndex) {
    if (tabIndex === 0) {
      this.activeTab.emit('alerts');
    } else {
      this.activeTab.emit('events');
    }
  }

  selectedIndexChangeEvent(event) {
    this.tabIndex = event;
    this.emitActiveTab(event);
  }
}
