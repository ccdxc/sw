import { Component, ViewChild, ViewEncapsulation, OnInit, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { MetricsqueryService, TelemetryPollingMetricQueries } from '@app/services/metricsquery.service';
import { ITelemetry_queryMetricsQueryResponse } from '@sdk/v1/models/telemetry_query';
import { ChartOptions } from 'chart.js';
import { Observer, Subject, Subscription } from 'rxjs';
import { sourceFieldKey, getFieldData } from './utility';
import { MetricMeasurement, MetricsMetadata } from '@sdk/metrics/generated/metadata';
import { ChartData, ChartDataSets, ColorTransform, DisplayLabelTransform, GroupByTransform, DataSource, MetricTransform, TransformDataset, TransformDatasets, GraphTransform } from './transforms';
import { AxisTransform } from './transforms/axis.transform';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { UIChartComponent } from '@app/components/shared/primeng-chart/chart';

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
 */


@Component({
  selector: 'app-telemetry',
  templateUrl: './telemetry.component.html',
  styleUrls: ['./telemetry.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: [Animations]
})
export class TelemetryComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('pChart') chartContainer: UIChartComponent;

  lineData: ChartData = {
    datasets: []
  };

  // The metric source that is currently selected for editing
  selectedDataSourceIndex: number;
  dataSources: DataSource[] = [];

  graphTypeOptions = [{
    label: 'Line Graph',
    value: 'Line Graph'
  }];
  selectedGraphType = 'Line Graph';

  metricsMetadata: { [key: string]: MetricMeasurement } = MetricsMetadata;
  measurements: MetricMeasurement[] = [];

  graphOptions: ChartOptions = this.generateDefaultGraphOptions();

  // Holds last response from the metric poll
  metricData: ITelemetry_queryMetricsQueryResponse;

  // Subscription for the metric polling
  metricSubscription: Subscription;
  subscriptions: Subscription[] = [];

  // Subject to be passed into data sources so that they can request data
  getMetricsSubject: Subject<any> = new Subject<any>();

  graphTransforms: GraphTransform[] = [
    new AxisTransform(),
  ];

  activeTabNumber = 0;

  selectedTimeRange: TimeRange;

  constructor(protected controllerService: ControllerService,
    protected telemetryqueryService: MetricsqueryService) {
      super(controllerService);
  }

  ngOnInit() {
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Telemetry', url: Utility.getBaseUIUrl() + 'monitoring/telemetry' }]
    });

    this.measurements = [];
    Object.keys(this.metricsMetadata).forEach( (m) => {
      this.measurements.push(this.metricsMetadata[m]);
    });

    const metricsSubjectSubscription = this.getMetricsSubject.subscribe( () => {
      this.getMetrics();
    });
    this.subscriptions.push(metricsSubjectSubscription);

    if (this.dataSources.length === 0) {
      this.addDataSource();
      this.selectedDataSourceIndex = 0;
    }
  }

  generateDefaultGraphOptions(): ChartOptions {
    return {
      title: {
          display: false,
      },
      legend: {
          display: true,
          position: 'bottom'
      },
      layout: {
        padding: 20
      },
      animation: {
        duration: 0
      },
      scales: {
        xAxes: [{
          type: 'time',
          display: true,
          gridLines: {
            display: true
          },
          scaleLabel: {
            display: true
          },
          ticks: {
            display: true,
          },
        }],
        yAxes: [{
          gridLines: {
            display: true
          },
          display: true,
          scaleLabel: {
            display: true
          },
          ticks: {
            display: true,
          }
        }]
      }
    };
  }

  getSelectedSource(): DataSource {
    if (this.selectedDataSourceIndex == null) {
      return null;
    }
    return this.dataSources[this.selectedDataSourceIndex];
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
    if (this.dataSources.length === 0 || this.dataSources[0] == null || this.dataSources[0].fields.length === 0) {
      return false;
    }
    return true;
  }

  addDataSource() {
    const source = new DataSource(this.getMetricsSubject as Observer<any>, [
      new DisplayLabelTransform(),
      new ColorTransform(),
      new GroupByTransform(),
    ]);
    this.dataSources.push(source);
  }

  removeDataSource(index: number) {
    if (this.dataSources.length > 1) {
      this.dataSources.splice(index, 1);
    }
    if (this.selectedDataSourceIndex === index) {
      this.selectedDataSourceIndex = null;
    } else if (this.selectedDataSourceIndex > index) {
      this.selectedDataSourceIndex -= 1;
    }
  }

  setTimeRange(timeRange: TimeRange) {
    this.selectedTimeRange = timeRange;
    this.getMetrics();
  }

  getMetrics() {
    if (this.metricSubscription) {
      this.metricSubscription.unsubscribe();
      this.metricSubscription = null;
    }
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    this.dataSources.forEach( (source) => {
      if (source.fields != null && source.fields.length !== 0) {
        const query = MetricsUtility.timeSeriesQueryPolling(source.measurement);
        // Set timerange
        query.query['start-time'] = this.selectedTimeRange.getTime().startTime.toISOString() as any;
        query.query['end-time'] = this.selectedTimeRange.getTime().endTime.toISOString() as any;
        source.transformQuery({query: query.query});
        queryList.queries.push(query);
      }
    });
    if (queryList.queries.length === 0) {
      this.metricData = null;
      this.drawGraph();
      return;
    }
    this.metricSubscription = this.telemetryqueryService.pollMetrics('telemetryExplore', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        // If tenant is null, then it is the default response from the behavior subject, and not from the backend
        if (data.tenant != null) {
          this.metricData = data;
          this.drawGraph();
        }
      }
    );
  }

  resetGraph() {
    this.lineData = { datasets: [] };
  }

  drawGraph() {
    if (this.metricData == null) {
      this.resetGraph();
      return;
    }

    if (!MetricsUtility.responseHasData(this.metricData)) {
      this.resetGraph();
      return;
    }

    // User can hide a dataset by clicking it's name on the legend
    // Create a map of which datasets are currently hidden, so that we can keep it.
    const hiddenDatasets = {};
    this.lineData.datasets.forEach( (dataset, index) => {

      if (!this.chartContainer.chart.isDatasetVisible(index)) {
        const key = sourceFieldKey(dataset.sourceID, dataset.sourceMeasurement, dataset.sourceField);
        hiddenDatasets[key] = true;
      }
    });

    // Start with a clean array so that we don't accidentally
    // have stale state transfer from the old options
    // All transforms should be relatively stateless
    const newGraphOptions: ChartOptions = this.generateDefaultGraphOptions();

    // Will be passed to graph transforms
    const allResultsDatasets = [];

    const resDataSets: ChartDataSets[] = [];
    this.metricData.results.forEach( (res, index) => {
      if (!MetricsUtility.resultHasData(res)) {
        // TODO: add only in legend
        return;
      }
      // Should be one to one with response length
      const source = this.dataSources[index];
      const singleResultDatasets: TransformDatasets = [];
      res.series.forEach( (s) => {
        source.fields.forEach( (field) => {
          const fieldIndex = s.columns.findIndex((f) => {
            return f.includes(field);
          });
          const data = Utility.transformToChartjsTimeSeries(s.values, 0, fieldIndex);
          const key = sourceFieldKey(source.id, source.measurement, field);
          const dataset: ChartDataSets = {
            data: data,
            pointRadius: 0,
            lineTension: 0,
            fill: false,
            sourceID: source.id,
            sourceField: field,
            sourceMeasurement: source.measurement,
            // If the dataset is currently being hidden, we continue to hide it
            hidden: hiddenDatasets[key] != null ? true : false,
          };
          const opt: TransformDataset = {
            dataset: dataset,
            series: s,
            measurement: source.measurement,
            field: field,
            // We put unit in here. Transforms are allowed
            // to change the unit type (kb -> mb), and should update this unit
            // property if they do
            units: getFieldData(source.measurement, field).units,
            fieldIndex: fieldIndex
          };
          source.transformDataset(opt);
          singleResultDatasets.push(opt);
          resDataSets.push(dataset);
        });
      });
      source.transformDatasets(singleResultDatasets);
      singleResultDatasets.forEach( (opt) => {
          allResultsDatasets.push(opt);
        }
      );
    });

    this.graphTransforms.forEach( (t) => {
      t.transformGraphOptions({
        data: allResultsDatasets,
        graphOptions: newGraphOptions,
        oldGraphOptions: this.graphOptions,
      });
    });

    this.graphOptions = newGraphOptions;

    this.lineData = {
      datasets: resDataSets
    };
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    if (this.metricSubscription != null) {
      this.metricSubscription.unsubscribe();
    }
  }

  // ------ HTML functions ------

  addDataSourceClick() {
    if (this.canAddDataSource()) {
      this.addDataSource();
      this.selectedDataSourceIndex = this.dataSources.length - 1;
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
