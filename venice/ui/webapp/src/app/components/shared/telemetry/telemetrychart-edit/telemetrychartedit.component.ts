import { Component, EventEmitter, Input, OnDestroy, OnInit, Output, ViewChild, ViewEncapsulation, AfterViewInit } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { GraphConfig } from '@app/models/frontend/shared/userpreference.interface';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { UIConfigsService, Features } from '@app/services/uiconfigs.service';
import { MetricsqueryService } from '@app/services/metricsquery.service';
import { MetricMeasurement, MetricsMetadata } from '@sdk/metrics/generated/metadata';
import { Subject, Subscription } from 'rxjs';
import { TelemetrychartComponent } from '../telemetrychart/telemetrychart.component';
import { DataSource, TransformNames } from '../transforms';
import { GraphTitleTransform } from '../transforms/graphtitle.transform';

/**
 * A data source allows a user to select a single measurement,
 * multiple fields, and other options such as filtering.
 *
 * A data source translates to one or more telemetry queries.
 * Each data source has a set of transforms registered. These transforms will
 * have their hooks called, and are able to modify the telemetry query, as well
 * as the response data set.
 *
 * Each transform should ideally be independent of each other, but it is
 * possible for transforms to read state from each other.
 *
 * Graph transforms are similar, but only affect graph level options.
 *
 * To see methods and hooks that transforms have access to, look at transforms/types.ts
 *
 */


@Component({
  selector: 'app-telemetrychartedit',
  templateUrl: './telemetrychartedit.component.html',
  styleUrls: ['./telemetrychartedit.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: [Animations]
})
export class TelemetrycharteditComponent extends BaseComponent implements OnInit, AfterViewInit, OnDestroy {
  @ViewChild('chart') chart: TelemetrychartComponent;
  @Input() chartConfig: GraphConfig;
  @Input() selectedTimeRange: TimeRange;
  @Output() saveChartReq: EventEmitter<GraphConfig> = new EventEmitter<GraphConfig>();
  @Output() cancelEdit: EventEmitter<any> = new EventEmitter<any>();

  // The metric source that is currently selected for editing
  selectedDataSourceIndex: number = 0;

  graphTypeOptions = [{
    label: 'Line Graph',
    value: 'Line Graph'
  }];
  selectedGraphType = 'Line Graph';

  metricsMetadata: { [key: string]: MetricMeasurement } = MetricsMetadata;
  measurements: MetricMeasurement[] = [];

  // Subscription for the metric polling
  subscriptions: Subscription[] = [];

  // Subject to be passed into data sources so that they can request data
  getMetricsSubject: Subject<any> = new Subject<any>();

  activeTabNumber: number = 0;

  graphTitle: string = '';

  showDebug: boolean = false;

  isToShowDebugMetric: boolean = false;

  constructor(protected controllerService: ControllerService,
    protected clusterService: ClusterService,
    protected authService: AuthService,
    protected uiConfigsService: UIConfigsService,
    protected telemetryqueryService: MetricsqueryService) {
      super(controllerService, uiConfigsService);
  }

  ngOnInit() {
    const buttons = [
      {
        cssClass: 'global-button-primary telemetry-button',
        text: 'SAVE CHART',
        computeClass: () => this.canChangeGraphOptions() ? '' : 'global-button-disabled',
        callback: () => { this.chart.saveConfig(); }
      },
      {
        cssClass: 'global-button-neutral telemetry-button',
        text: 'CANCEL',
        callback: () => { this.cancelEdit.emit(); }
      },

    ];
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Metrics', url: Utility.getBaseUIUrl() + 'monitoring/metrics' }]
    });
    this.measurements = [];
    Object.keys(this.metricsMetadata).forEach( (m) => {
      this.measurements.push(this.metricsMetadata[m]);
    });
  }


  ngAfterViewInit() {
    // Pushing to next event loop so that updating the graphTitle doesn't cause an angular change detection error.
    setTimeout(() => {
      const transform = this.chart.graphTransforms.find(x => x.transformName === TransformNames.GraphTitle) as GraphTitleTransform;
      if (transform != null) {
        this.graphTitle = transform.title.value;
        const sub = transform.title.valueChanges.subscribe(
          (value) => {
            this.graphTitle = value;
          }
        );
        this.subscriptions.push(sub);
      }
      // PS-1956 we what to hide "debug metrics" checkbox.
      this.isToShowDebugMetric = this.uiconfigsService.isFeatureEnabled(Features.showDebugMetrics);
    }, 0);
  }

  addDataSource(): DataSource {
    return this.chart.addDataSource();
  }

  removeDataSource(index: number) {
    this.chart.removeDataSource(index);
    if (this.selectedDataSourceIndex === index) {
      this.selectedDataSourceIndex = null;
    } else if (this.selectedDataSourceIndex > index) {
      this.selectedDataSourceIndex -= 1;
    }
  }

  debugMetricsChange() {
    if (this.showDebug) {
      // Don't need to uncheck anything
      return;
    }
    // If the user has checked a debug metric, when we turn off debug metrics
    // we should clear their selections
    const source = this.getSelectedSource();
    if (source.measurement == null) {
      return;
    }
    if (MetricsMetadata[source.measurement].tags == null || MetricsMetadata[source.measurement].tags.includes('Level7')) {
      // Need to remove selection
      source.measurement = null;
    }
  }

  getSelectedSource(): DataSource {
    if (this.chart == null || this.chart.dataSources.length  === 0) {
      return null;
    }
    return this.chart.dataSources[this.selectedDataSourceIndex];
  }

  // User is allowed to add a new data source if there are no incomplete data sources
  canAddDataSource(): boolean {
    const current = this.getSelectedSource();
    if (current != null && current.fields.length === 0) {
      // Incomplete source, don't allow them to add another
      return false;
    }
    return true;
  }

  // Whether or not user is allowed to switch to the graph options tab
  canChangeGraphOptions(): boolean {
    if (this.chart.dataSources.length === 0 || this.chart.dataSources[0] == null || this.chart.dataSources[0].fields.length === 0) {
      return false;
    }
    return true;
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  // ------ HTML functions ------

  addDataSourceClick() {
    if (this.canAddDataSource()) {
      this.addDataSource();
      this.selectedDataSourceIndex = this.chart.dataSources.length - 1;
    }
  }

  viewSourceClick(index: number) {
    // If the current source we are editing doesn't have a
    // measurement picked, we remove it
    const indexToRemove = this.selectedDataSourceIndex;
    const currentSource = this.getSelectedSource();
    this.selectedDataSourceIndex = index;
    if (currentSource != null && currentSource.fields.length === 0) {
      this.removeDataSource(indexToRemove);
    }
  }

  deleteSourceClick($event, index) {
    $event.stopPropagation(); // prevents the source from trying to expand
    this.removeDataSource(index);
  }

  graphTypeChange() {}

  getMeasurementMetadata(measurement: string): MetricMeasurement {
    return MetricsMetadata[measurement];
  }

  getFieldMetadata(measurement: string, field: string) {
    const fields = this.metricsMetadata[measurement].fields;
    if (fields != null) {
      return fields.find( (f) => {
        return f.name === field;
      });
    }
    return null;
  }

}
