import { Component, OnDestroy, OnInit, QueryList, ViewChildren, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { DashboardWidgetData, PinnedDashboardWidgetData } from '@app/models/frontend/dashboard/dashboard.interface';
import { BaseComponent } from '@components/base/base.component';

import { ControllerService } from '../../services/controller.service';

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

  @ViewChildren('pinnedGridster') pinnedGridster: QueryList<any>;

  constructor(protected _controllerService: ControllerService,
  ) {
    super(_controllerService);
  }

  /**
   * Component enters init stage. It is about to show up
   */
  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'DashboardComponent', 'state': Eventtypes.COMPONENT_INIT });
      this._controllerService.setToolbarData({
        buttons: [
          {
            cssClass: 'global-button-primary dbsd-refresh-button',
            text: 'Refresh',
            callback: () => { this.getDefaultDashboardWidgets(); },
          }],
        breadcrumb: [{ label: 'Dashboard', url: '' }]
      });

      this.pinnedGridsterOptions = Object.assign({}, this.gridsterOptions);
      this.pinnedGridsterOptions.maxRows = 1;

      this.getDefaultDashboardWidgets();
      this.getPinnedData();
    }
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
