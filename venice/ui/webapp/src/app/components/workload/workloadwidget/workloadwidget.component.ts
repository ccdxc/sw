import { Component, OnInit, OnDestroy, ViewEncapsulation, Input, OnChanges, Output, EventEmitter } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/workload.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Subscription } from 'rxjs/Subscription';

import { PinPayload } from '@app/models/frontend/shared/pinpayload.interface';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-workloadwidget',
  templateUrl: './workloadwidget.component.html',
  styleUrls: ['./workloadwidget.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadwidgetComponent extends BaseComponent implements OnInit, OnDestroy, OnChanges {
  moduleName = '@components/workload/workload.module#WorkloadModule';

  isPinned: boolean;
  private subscription: Subscription;
  arrowDir: string;
  _iconStyles: any;
  _iconPinnedStyles: any;
  dataset: any;
  color: string;
  style_class: string;
  widgetHover = false;
  pinHover = false;
  onDashboard: boolean;

  data: any;
  layout: any; // Plotly layout
  options: any; // plotly options
  title: string;
  label: string;
  icon: Icon;
  styleType: string; // For setting color to be positive or negative theme
  imageWidth = 176;
  imageHeight = 23.5;
  pinPayload: PinPayload;
  isReady = false; // Used for determining when to remove spinner

  @Input() id: string;
  @Input() selected: string;
  @Input() collapsedView: boolean;

  @Output() workloadWidgetClick: EventEmitter<any> = new EventEmitter();
  constructor(private _workloadService: WorkloadService,
    protected _controllerService: ControllerService) {
    super(_controllerService);
  }

  ngOnInit() {
    if (this.id) {
      this.setupWidget();
    }
  }

  setupWidget() {
    this.pinPayload = {
      module: this.moduleName,
      component: this.getClassName(),
      args: {
        id: this.id
      }
    };
    this.isPinned = this._controllerService.alreadyPinned(this.pinPayload);
    this.getWidgetData();
  }

  dashboardSetup() {
    this.onDashboard = true;
  }

  getDashboardSettings() {
    return {
      rows: 1,
      cols: 1,
    };
  }

  getWidgetData() {
    const staticData = this._workloadService.getStaticWidgetDataById(this.id);
    if (staticData != null) {
      this.icon = staticData.icon;
      this.title = staticData.title;
    }

    this.subscription = this._workloadService.getWidgetDataById(this.id).subscribe(data => {
      if (Utility.isRESTSuccess(data)) {
        this.data = data.data;
        this.label = data.label;
        this.styleType = data.style;
        this.arrowDir = data.arrow;
        if (this.data != null) {
          this.updateData();
        }
      }
    });
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setupStyle() {
    // setting color options
    if (this.styleType === 'positive') {
      this.style_class = 'wlWidget-positive';
      this.color = '#7db1ea';
      this._iconPinnedStyles = {
        'background-color': 'rgba(125,177,234, 0.2)'
      };
    } else if (this.styleType === 'negative') {
      this.style_class = 'wlWidget-negative';
      this.color = '#e57553';
      this._iconPinnedStyles = {
        'background-color': 'rgba(229,117,83, 0.2)'
      };
    } else {
      this.style_class = 'wlWidget-neutral';
      this.color = '#000000';
    }
  }

  updateData() {
    this._iconStyles = this._setIconStyles();
    this.setupStyle();
    this._setupGraphOptions();
    this.isReady = true;
  }

  protected _setupData() {
    if (!this.data) {
      this.data = {
        x: [1, 2, 3, 4, 5],
        y: [1, 2, 3, 4, 5]
      };
    }
    this.dataset = [
      // passed in graph data
      {
        x: this.data.x,
        y: this.data.y,
        type: 'scatter',
        mode: 'lines',
        line: {
          shape: 'linear',
          color: this.color,
          width: 1
        }
      },
      // second trace to add marker on last point
      {
        x: [this.data.x[this.data.x.length - 1]],
        y: [this.data.y[this.data.y.length - 1]],
        type: 'scatter',
        mode: 'marker',
        line: {
          color: this.color,
        }
      }
    ];
  }

  protected _setupLayout() {
    this.layout = {
      showlegend: false,
      paper_bgcolor: 'rgba(0,0,0,0)',
      plot_bgcolor: 'rgba(0,0,0,0)',
      autosize: true,
      margin: {
        t: 0,
        l: 0,
        r: 0,
        b: 0
      },
      yaxis: {
        autorange: true,
        showgrid: false,
        zeroline: false,
        showline: false,
        autotick: true,
        ticks: '',
        showticklabels: false
      },
      xaxis: {
        autorange: true,
        showgrid: false,
        zeroline: false,
        showline: false,
        autotick: true,
        ticks: '',
        showticklabels: false
      }
    };
  }

  protected _setupGraphOptions() {
    // if passed in data is array, assume already formatted
    // for use with plotly
    if (!Array.isArray(this.data)) {
      // assume passed in data is JSON of x and y points
      this._setupData();
    } else {
      this.dataset = this.data;
    }
    if (!this.layout) {
      this._setupLayout();
    }
    if (!this.options) {
      this.options = { displayModeBar: false };
    }
  }

  _setIconStyles() {
    if (!this.icon) {
      return '';
    }
    const styles = {
      'margin-top': this.icon.margin.top,
      'margin-right': this.icon.margin.right,
    };
    return styles;
  }

  ngOnDestroy() {
    if (this.subscription != null) {
      this.subscription.unsubscribe();
    }
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'WorkloadwidgetComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }

  ngOnChanges() {
    if (this.isReady) {
      this.updateData();
    }
  }

  handlePinClick($event) {
    $event.stopPropagation();
    if (this.isPinned) {
      this._controllerService.publish(Eventtypes.UNPIN_REQUEST, this.pinPayload);
      this.isPinned = false;
    } else {
      this._controllerService.publish(Eventtypes.PIN_REQUEST, this.pinPayload);
      this.isPinned = true;
    }
  }

  itemClick($event) {
    this.workloadWidgetClick.emit(this.id);
  }
}
