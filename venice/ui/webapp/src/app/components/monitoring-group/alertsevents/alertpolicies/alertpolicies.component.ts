import { Component, OnInit, ViewEncapsulation, OnDestroy } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { BaseComponent } from '@app/components/base/base.component';
import { IMonitoringAlertDestination, IMonitoringAlertPolicy, IApiStatus, MonitoringAlertPolicy, MonitoringAlertDestination } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Subscription } from 'rxjs';
import { Utility } from '@app/common/Utility';
import {Router} from '@angular/router';

@Component({
  selector: 'app-alertpolicies',
  templateUrl: './alertpolicies.component.html',
  styleUrls: ['./alertpolicies.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class AlertpoliciesComponent extends BaseComponent implements OnInit, OnDestroy {
  eventPolicies: ReadonlyArray<IMonitoringAlertPolicy> = [];
  metricPolicies: ReadonlyArray<IMonitoringAlertPolicy> = [];
  objectPolicies: ReadonlyArray<IMonitoringAlertPolicy> = [];
  destinations: ReadonlyArray<IMonitoringAlertDestination> = [];

  eventPoliciesEventUtility: HttpEventUtility<MonitoringAlertPolicy>;
  metricPoliciesEventUtility: HttpEventUtility<any>;
  objectPoliciesEventUtility: HttpEventUtility<any>;
  destinationsEventUtility: HttpEventUtility<MonitoringAlertDestination>;

  subscriptions: Subscription[] = [];
  selectedIndex = 0;

  constructor(protected _controllerService: ControllerService,
    protected _monitoringService: MonitoringService,
    protected router: Router
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this.activeTab();
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
      'component': 'AlertpoliciesComponent', 'state':
        Eventtypes.COMPONENT_INIT
    });
    this.getAlertPolicies();
    this.getDestinations();
    this.updateBreadCrumb(0);
  }

  updateBreadCrumb(tabindex: number) {
    this._controllerService.setToolbarData({
      buttons: [
      ],
      breadcrumb: [
        { label: 'Alerts & Events', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents' },
        { label: this.getSecondCrumbLabel(tabindex), url: Utility.getBaseUIUrl() + this.getSecondCrumbUrl(tabindex) }
      ]
    });
  }

  getSecondCrumbLabel(tabindex: number) {
    if (tabindex === 0) {
      return 'Alert Policies';
    } else {
      return 'Destinations';
    }
  }

  getSecondCrumbUrl(tabindex: number) {
    if (tabindex === 0) {
      return 'monitoring/alertsevents/alertpolicies';
    } else {
      return 'monitoring/alertsevents/alertdestinations';
    }
  }

  tabSwitched(event) {
    if (event === 0) {
      this.router.navigate(['/monitoring/alertsevents/alertpolicies']);
    } else {
      this.router.navigate(['/monitoring/alertsevents/alertdestinations']);
    }
    this.activeTab();
    this.updateBreadCrumb(event);
  }

  activeTab() {
    if (this.router.url.endsWith('alertdestinations')) {
      this.selectedIndex = 1;
    } else {
      this.selectedIndex = 0;
    }
  }


  /**
  * Overide super's API
  * It will return this Component name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  getAlertPolicies() {
    this.eventPoliciesEventUtility = new HttpEventUtility<MonitoringAlertPolicy>(MonitoringAlertPolicy, false,
      (policy) => {
        return policy.spec.resource === 'Event';
      }
    );
    this.metricPoliciesEventUtility = new HttpEventUtility<any>(null, false,
      (policy) => {
        return policy.spec.resource === 'EndpointMetrics';
      }
    );
    this.objectPoliciesEventUtility = new HttpEventUtility<any>(null, false,
      (policy) => {
        return policy.spec.resource !== 'EndpointMetrics' &&
          policy.spec.resource !== 'Event';
      }
    );
    this.eventPolicies = this.eventPoliciesEventUtility.array;
    this.metricPolicies = this.metricPoliciesEventUtility.array;
    this.objectPolicies = this.objectPoliciesEventUtility.array;
    const subscription = this._monitoringService.WatchAlertPolicy().subscribe(
      (response) => {
        this.eventPoliciesEventUtility.processEvents(response);
        this.metricPoliciesEventUtility.processEvents(response);
        this.objectPoliciesEventUtility.processEvents(response);
      },
      this._controllerService.webSocketErrorHandler('Failed to get Alert Policies')
    );
    this.subscriptions.push(subscription);
  }

  getDestinations() {
    this.destinationsEventUtility = new HttpEventUtility<MonitoringAlertDestination>(MonitoringAlertDestination);
    this.destinations = this.destinationsEventUtility.array;
    const subscription = this._monitoringService.WatchAlertDestination().subscribe(
      (response) => {
        this.destinationsEventUtility.processEvents(response);
      },
      this._controllerService.webSocketErrorHandler('Failed to get Alert Destinations')
    );
    this.subscriptions.push(subscription);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'AlertpoliciesComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }


}
