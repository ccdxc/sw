import { Component, OnDestroy, OnInit, QueryList, ViewChildren, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { DashboardWidgetData, PinnedDashboardWidgetData } from '@app/models/frontend/dashboard/dashboard.interface';
import { BaseComponent } from '@components/base/base.component';

import { ControllerService } from '../../services/controller.service';
import { MetricsqueryService, TelemetryPollingMetricQueries, MetricsPollingQuery, MetricsPollingOptions } from '@app/services/metricsquery.service';
import { Telemetry_queryMetricsQuerySpec } from '@sdk/v1/models/generated/telemetry_query';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { CardStates } from '../shared/basecard/basecard.component';

/**
 * This is Dashboard Component for VeniceUI
 * Dashboard.component.html has two gridster widgets and has multiple ng-template registered.  This component's widgets[] will map to ng-template by "id" field.
 *
 * We use gridster widget to enable grid layout and enable grid-item drag-and-drop
 * DashboardComponent.gridsterOptions defines the grid (2x6) matrix. Each cell is XXXpx height. css .dsbd-main-gridster-content defines the gridster height as YYY.
 * Thus, we must be careful to create widget which will be hosted in a grid-item in XXXpx height.
 *
 * We use canvas API to draw extra text in charts.  We are using chart.js charts as it offers better animation look & feel.  See "Naple and Workload" configurations.
 *
 * Other UI pages may pin their widgets to Dashboard. getPinnedData() is the API that loads those pinned-in widgets and add to Dashboard.
 * The UI pages that invoke "pin-to-dashboard" operations are responsible for provide widget invokation seeting.  See workload.component.ts for how it is done.
 *
 * Note:
 * To enable chart animation, we use gridster.itemInitCallback() api.  HTML template includes '*ngIf="item.dsbdGridsterItemIsReady" ' to manage timing.
 * Dashboard includes default widgets, such as "system capacity", "software version", etc.
 * Object in _buildSystemCapacity() is carefully wired to expected configuration of systemcapacity.component.ts
 *
 */

@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class DashboardComponent extends BaseComponent implements OnInit, OnDestroy {
  subscriptions = [];
  gridsterOptions: any = {
    gridType: 'fixed',
    fixedRowHeight: 275,
    fixedColWidth: 275,
    compactType: 'compactUp&Left',
    margin: 5,
    draggable: {
      enabled: true,
    },
    itemInitCallback: this.gridsterItemInitCallback,
    displayGrid: 'none'
  };
  pinnedGridsterOptions: any = {};
  title = 'Venice UI Dashboard';

  widgets: DashboardWidgetData[];
  pinnedWidgets: PinnedDashboardWidgetData[];

  lastUpdateTime: string = '';

  timeSeriesData: ITelemetry_queryMetricsQueryResult;
  currentData: ITelemetry_queryMetricsQueryResult;
  prevData: ITelemetry_queryMetricsQueryResult;
  avgDayData: ITelemetry_queryMetricsQueryResult;

  systemCapacity = {
    cardState: CardStates.LOADING
  };

  @ViewChildren('pinnedGridster') pinnedGridster: QueryList<any>;

  constructor(protected _controllerService: ControllerService,
    protected metricsqueryService: MetricsqueryService,
  ) {
    super(_controllerService);
  }

  /**
   * Component enters init stage. It is about to show up
   */
  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'DashboardComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._controllerService.setToolbarData({
      buttons: [
        // Commenting out for now as they are not hooked up
        // {
        //   cssClass: 'global-button-primary dbsd-refresh-button',
        //   text: 'Help',
        //   callback: () => {
        //     console.log(this.getClassName() + 'toolbar button help call()');
        //   }
        // }
      ],
      splitbuttons: [
        // Commenting out for now as they are not hooked up
        // {
        //   text: 'Refresh',
        //   icon: 'pi pi-refresh',
        //   callback: (event, sbutton) => {
        //     console.log('dashboard toolbar splitbutton refresh');
        //   },
        //   items: [
        //     {
        //       label: '5 days', icon: 'pi pi-cog', command: () => {
        //         console.log('dashboard toolbar menuitem 5-days');
        //       }
        //     },
        //     {
        //       label: '10 days', icon: 'pi pi-times', command: () => {
        //         console.log('dashboard toolbar menuitem 10-days');
        //       }
        //     }
        //   ]
        // }
      ],
      breadcrumb: [{ label: 'Dashboard', url: Utility.getBaseUIUrl() + 'dashboard' }]
    });

    this.pinnedGridsterOptions = Object.assign({}, this.gridsterOptions);
    this.pinnedGridsterOptions.maxRows = 1;

    this.getDefaultDashboardWidgets();
    this.getPinnedData();

    this.getSystemCapacityMetrics();
  }

  /**
   * Used for alerting the item content when its surrounding gridster
   * is ready. Allows animations to happen after the widget is viewable.
   */
  gridsterItemInitCallback(gridsterItem, gridsterItemComponent) {
    gridsterItem.dsbdGridsterItemIsReady = true;
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to exist
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'dashboardComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  timeSeriesQuery(): MetricsPollingQuery {
    // TODO: Optimize query. Don't pull all stats
    return MetricsUtility.timeSeriesQueryPolling('Node', []);
  }

  currentQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastFiveMinAverageQuery('Node');
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastFiveMinQueryUpdate,
    };

    return { query: query, pollingOptions: pollOptions };
  }

  // Gets between 10 to 5 min ago
  prevQuery(): MetricsPollingQuery {
    const start = 'now() - 10m';
    const end = 'now() - 5m';
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.intervalAverageQuery('Node', start, end);
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.genIntervalAverageQueryUpdate(start, end),
    };

    return { query: query, pollingOptions: pollOptions };
  }

  avgDayQuery(): MetricsPollingQuery {
    const query: Telemetry_queryMetricsQuerySpec = MetricsUtility.pastDayAverageQuery('Node');
    const pollOptions: MetricsPollingOptions = {
      timeUpdater: MetricsUtility.pastDayAverageQueryUpdate,
    };
    return { query: query, pollingOptions: pollOptions };
  }

  getSystemCapacityMetrics() {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.timeSeriesQuery());
    queryList.queries.push(this.currentQuery());
    queryList.queries.push(this.prevQuery());
    queryList.queries.push(this.avgDayQuery());
    const sub = this.metricsqueryService.pollMetrics('dsbdCards', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        if (data && data.results && data.results.length === queryList.queries.length) {
          if (MetricsUtility.resultHasData(data.results[1])) {
            this.timeSeriesData = data.results[0];
            this.currentData = data.results[1];
            this.prevData = data.results[2];
            this.avgDayData = data.results[3];
            this.lastUpdateTime = new Date().toISOString();
            this.enableSystemCapacityCard();
          } else {
            this.enableSystemCapacityNoData();
          }
        }
      },
      (err) => {
        this.setSystemCapacityErrorState();
      }
    );
    this.subscriptions.push(sub);
  }

  enableSystemCapacityCard() {
    this.systemCapacity.cardState = CardStates.READY;
  }

  setSystemCapacityErrorState() {
    this.systemCapacity.cardState = CardStates.FAILED;
  }

  enableSystemCapacityNoData() {
    this.systemCapacity.cardState = CardStates.NO_DATA;
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getPinnedData() {
    this.pinnedWidgets = [];
    const $ = Utility.getJQuery();
    const items = this._controllerService.getPinnedDashboardItems();

    items.forEach(elem => {
      this._controllerService.buildComponentFromModule(elem.module,
        elem.component)
        .then((component: any) => {
          // change component properties and append component to UI view
          for (const key of Object.keys(elem.args)) {
            component.instance[key] = elem.args[key];
          }
          component.instance.dashboardSetup();
          const settingsWidget = component.instance.getDashboardSettings();
          settingsWidget.id = 'gridsterItem' + component.instance.id;
          settingsWidget.initCallback = () => {
            this._controllerService.appendComponentToDOMElement(component, $('#' + settingsWidget.id).get(0));
          };
          this.pinnedWidgets.push(settingsWidget);
        });
    });
  }

  private _buildSoftwareVersion(): DashboardWidgetData {
    return {
      id: 'software_version',
      x: 4, y: 0,
      // w: 2, h: 1,
      rows: 1, cols: 1,
      dsbdGridsterItemIsReady: false,
    };
  }

  private _buildSystemCapacity(): DashboardWidgetData {
    const ret = {
      x: 0, y: 0,
      rows: 1, cols: 2,
      dsbdGridsterItemIsReady: false,
      id: 'system_capacity',
    };
    return ret;
  }

  private _buildPoliciesHealth(): DashboardWidgetData {
    return {
      x: 3, y: 1,
      rows: 1, cols: 2,
      dsbdGridsterItemIsReady: false,
      id: 'policy_health',
    };
  }

  private _buildWorkloads(): DashboardWidgetData {
    return {
      x: 2, y: 0,
      rows: 1, cols: 2,
      dsbdGridsterItemIsReady: false,
      id: 'workloads',
    };
  }

  private _buildNaples(): DashboardWidgetData {
    return {
      x: 1, y: 0,
      rows: 1, cols: 3,
      dsbdGridsterItemIsReady: false,
      id: 'naples',
    };
  }

  protected getDefaultDashboardWidgets() {
    if (!this.widgets) {
      this.widgets = [];
    } else {
      this.widgets.length = 0;
    }
    this.widgets.push(this._buildSystemCapacity());
    this.widgets.push(this._buildWorkloads());
    this.widgets.push(this._buildNaples());
    this.widgets.push(this._buildPoliciesHealth());
    this.widgets.push(this._buildSoftwareVersion());
  }

  setWidth(widget: any, size: number, e: MouseEvent) {
    e.stopPropagation();
    e.preventDefault();
    if (size < 1) {
      size = 1;
    }
    widget.w = size;

    return false;
  }

  setHeight(widget: any, size: number, e: MouseEvent) {
    e.stopPropagation();
    e.preventDefault();
    if (size < 1) {
      size = 1;
    }
    widget.h = size;
    return false;
  }



}
