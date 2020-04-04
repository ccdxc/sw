import { Component, ViewChild, ViewEncapsulation, OnInit, OnDestroy, Input, Output, EventEmitter, SimpleChanges, OnChanges } from '@angular/core';
import { Animations } from '@app/animations';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { MetricsqueryService, TelemetryPollingMetricQueries, MetricsPollingQuery } from '@app/services/metricsquery.service';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { ChartOptions } from 'chart.js';
import { Observer, Subject, Subscription } from 'rxjs';
import { sourceFieldKey, getFieldData } from '../utility';
import { MetricMeasurement, MetricsMetadata } from '@sdk/metrics/generated/metadata';
import { ChartData, ChartDataSets, ColorTransform, DisplayLabelTransform, GroupByTransform, DataSource, MetricTransform, TransformDataset, TransformDatasets, GraphTransform } from '../transforms';
import { AxisTransform } from '../transforms/axis.transform';
import { TimeRange } from '@app/components/shared/timerange/utility';
import { UIChartComponent } from '@app/components/shared/primeng-chart/chart';
import { FieldSelectorTransform } from '../transforms/fieldselector.transform';
import { FieldValueTransform, ValueMap, QueryMap } from '../transforms/fieldvalue.transform';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterDistributedServiceCard, ClusterNode } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ITelemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQuerySpec_function } from '@sdk/v1/models/generated/telemetry_query';
import { LabelSelectorTransform } from '../transforms/labelselector.transform';
import { AuthService } from '@app/services/generated/auth.service';
import { TelemetryPref, GraphConfig, DataTransformConfig } from '@app/models/frontend/shared/userpreference.interface';
import * as moment from 'moment';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { GraphTitleTransform } from '../transforms/graphtitle.transform';
import { GroupByTimeTransform } from '../transforms/groupbytime.transform';
import { RoundCountersTransform } from '../transforms/roundcounters.transform';
import { debounceTime } from 'rxjs/operators';
import { DerivativeTransform } from '../transforms/derivative.transform';

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
  selector: 'app-telemetrychart',
  templateUrl: './telemetrychart.component.html',
  styleUrls: ['./telemetrychart.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: [Animations]
})
export class TelemetrychartComponent extends BaseComponent implements OnInit, OnChanges, OnDestroy {
  public static MAX_LEGEND_EDIT_MODE: number = 20;
  public static MAX_LEGEND_VIEW_MODE: number = 7;

  @ViewChild('pChart') chartContainer: UIChartComponent;
  @Input() chartConfig: GraphConfig;
  @Input() inEditMode: boolean = false;
  @Input() inDebugMode: boolean = false;
  @Input() selectedTimeRange: TimeRange;
  @Output() saveChartReq: EventEmitter<GraphConfig> = new EventEmitter<GraphConfig>();

  graphLoading: boolean = false;

  themeColor: string = '#b592e3';

  icon: Icon = {
    margin: {
      top: '8px',
      left: '0px'
    },
    matIcon: 'insert_chart_outlined'
  };

  telemetryPref: TelemetryPref = {
    items: [],
    configuredGraphs: {}
  };

  lineData: ChartData = {
    datasets: []
  };

  // The metric source that is currently selected for editing
  dataSources: DataSource[] = [];

  graphTypeOptions = [{
    label: 'Line Graph',
    value: 'Line Graph'
  }];
  selectedGraphType = 'Line Graph';

  metricsMetadata: { [key: string]: MetricMeasurement } = MetricsMetadata;

  graphOptions: ChartOptions = this.generateDefaultGraphOptions();

  // Holds last response from the metric poll
  metricData: ITelemetry_queryMetricsQueryResponse;

  // Subscription for the metric polling
  metricSubscription: Subscription;
  subscriptions: Subscription[] = [];

  // Metrics observer, used for debouncing metric requests
  metricsQueryObserver: Subject<TelemetryPollingMetricQueries> = new Subject();

  // Last query we sent to the poll utility
  lastQuery: TelemetryPollingMetricQueries = null;
  // last time range used for the last query
  lastTimeRange: TimeRange = null;

  // Subject to be passed into data sources so that they can request data
  getMetricsSubject: Subject<any> = new Subject<any>();

  // Subject to be passed into graph transforms so that they can request graph redraw
  reqRedrawSubject: Subject<any> = new Subject<any>();

  graphTransforms: GraphTransform<any>[] = [
    new GraphTitleTransform(),
    new AxisTransform()
    // comment out DerivativeTransform to avoid TCP Session to negative numbers
    // new DerivativeTransform(),
  ];

  naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  nodes: ReadonlyArray<ClusterNode> = [];
  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;
  nodeEventUtility: HttpEventUtility<ClusterNode>;

  macAddrToName: { [key: string]: string; } = {};
  nameToMacAddr: { [key: string]: string; } = {};

  // Map from object kind to map from key to map of values to object names
  // that have that label
  labelMap:
    { [kind: string]:
      { [labelKey: string]:
        { [labelValue: string]: string[] }
      }
    } = {};


  fieldQueryMap: QueryMap =  {};
  fieldValueMap: ValueMap = {};

  // Flag to indicate whether we have finished loading a user's config
  // Used to prevent fetching metrics before all configs are loaded
  configLoaded: boolean = false;

  constructor(protected controllerService: ControllerService,
    protected clusterService: ClusterService,
    protected authService: AuthService,
    protected telemetryqueryService: MetricsqueryService) {
      super(controllerService);
  }

  ngOnInit() {
    this.metricsQueryListener();
    this.setupValueOverrides();
    this.getNaples();
    this.getNodes();

    if (this.chartConfig == null) {
      this.chartConfig = {
        id: '',
        graphTransforms: {
          transforms: {},
        },
        dataTransforms: []
      };
    }

    const metricsSubjectSubscription = this.getMetricsSubject.subscribe( () => {
      if (this.configLoaded) {
        this.getMetrics();
      }
    });
    this.subscriptions.push(metricsSubjectSubscription);

    const reqDrawSub = this.reqRedrawSubject.subscribe( () => {
      this.drawGraph();
    });
    this.subscriptions.push(reqDrawSub);

    // Populating graph transform reqRedraw
    this.graphTransforms.forEach( (t) => {
      // Casting to any so we can set private variables
      (<any>t).reqRedraw = this.reqRedrawSubject;
    });

    if (this.chartConfig != null && this.chartConfig.id !== '') {
      this.loadConfig();
    }
    this.configLoaded = true;

    if (this.dataSources.length === 0) {
      this.addDataSource();
    }
  }

  setupValueOverrides() {
    const queryMapNaplesReporterID = (res: ITelemetry_queryMetricsQuerySpec) => {
      const field = 'reporterID';
      res.selector.requirements.forEach( (req) => {
        if (req.key === field) {
          req.values = req.values.map( (v) => {
            const mac = this.nameToMacAddr[v];
            if (mac != null) {
              return mac;
            }
            return v;
          });
        }
      });
    };
    const valueMapNaplesReporterID = (res: ITelemetry_queryMetricsQueryResult) => {
      const field = 'reporterID';
      res.series.forEach( (s) => {
        if (s.tags != null) {
          const tagVal = s.tags[field];
          if (tagVal != null && this.macAddrToName[tagVal] != null) {
            s.tags[field] = this.macAddrToName[tagVal];
          }
        }
        const fieldIndex = s.columns.findIndex((f) => {
          return f.includes(field);
        });
        s.values = s.values.map( (v) => {
          const mac = v[fieldIndex];
          if (this.macAddrToName[mac] != null) {
            v[fieldIndex] = this.macAddrToName[mac];
          }
          return v;
        });
      });
    };
    Object.keys(MetricsMetadata).forEach( (m) => {
      const measurement = MetricsMetadata[m];
      if (measurement.objectKind === 'DistributedServiceCard') {
        this.fieldQueryMap[measurement.name] = queryMapNaplesReporterID;
        this.fieldValueMap[measurement.name] = valueMapNaplesReporterID;
      }
    });
  }

  ngOnChanges(changes: SimpleChanges) {
    // If the time range is changed, we refetch metrics.
    // We don't want this to run on first change because the data sources
    // might not be ready yet.
    if (changes.selectedTimeRange != null && !changes.selectedTimeRange.isFirstChange()) {
      this.getMetrics();
    }
    if (changes.inDebugMode) {
      this.dataSources.forEach( (s) => {
        s.debugMode = this.inDebugMode;
      });
    }
  }

  loadConfig() {
    // loading graph transforms
    // We clone the preference before loading the config
    // so that incase the transform modifies the config,
    // if a user then cancels the graph edit the inital user
    // preference content is preserved
    const _ = Utility.getLodash();
    this.graphTransforms.forEach( (t) => {
      t.load(_.cloneDeep(this.chartConfig.graphTransforms.transforms[t.transformName]));
    });

    this.chartConfig.dataTransforms.forEach( (dataTransform) => {
      const source = this.addDataSource();
      source.load(_.cloneDeep(dataTransform));
    });
    this.configLoaded = true;
    this.getMetrics();
  }

  saveConfig() {
    // Saving graph config
    const graphTransforms = {};
    this.graphTransforms.forEach( (t) => {
      graphTransforms[t.transformName] = t.save();
    });
    this.chartConfig.graphTransforms.transforms = graphTransforms;
    const dataSourceConfig: DataTransformConfig[] = [];
    this.dataSources.forEach( source => {
      if (source.measurement == null || source.fields.length === 0) {
        return; // Don't save an incomplete query
      }
      dataSourceConfig.push(source.save());
    });
    this.chartConfig.dataTransforms = dataSourceConfig;
    // The chart id will be populated by telemetry.component.ts before saving.

    // emit back to parent, which has the full user pref object.
    this.saveChartReq.emit(this.chartConfig);
  }


  getNodes() {
    this.nodeEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.nodes = this.nodeEventUtility.array;
    const subscription = this.clusterService.WatchNode().subscribe(
      response => {
        this.nodeEventUtility.processEvents(response);
        this.labelMap['Node'] = Utility.getLabels(this.nodes as any[]);
        this.getMetrics();
      },
      this.controllerService.webSocketErrorHandler('Failed to get PSM nodes')
    );
    this.subscriptions.push(subscription);
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>(ClusterDistributedServiceCard);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    const subscription = this.clusterService.WatchDistributedServiceCard().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        this.labelMap['DistributedServiceCard'] = Utility.getLabels(this.naples as any[]);
        // mac-address to name map
        this.macAddrToName = {};
        this.nameToMacAddr = {};
        for (const nic of this.naples) {
          this.macAddrToName[nic.meta.name] = nic.spec.id;
          if (nic.spec.id != null || nic.spec.id.length > 0) {
            this.nameToMacAddr[nic.spec.id] = nic.meta.name;
          }
        }
        this.getMetrics();
      },
      this.controllerService.webSocketErrorHandler('Failed to get labels')
    );
    this.subscriptions.push(subscription);
  }

  checkIfQueriesSelectorChanged(newQuery: TelemetryPollingMetricQueries, oldQuery: TelemetryPollingMetricQueries): boolean {
    if (oldQuery == null || oldQuery.queries.length === 0 || newQuery == null || newQuery.queries.length === 0) {
      return false;
    }
    return oldQuery.queries.some((query, index) => {
      return !Utility.getLodash().isEqual(query.query.selector, newQuery.queries[index].query.selector);
    });
  }

  generateDefaultGraphOptions(): ChartOptions {
    const options: ChartOptions = {
      title: {
          display: false,
      },
      legend: {
          display: true,
          position: 'bottom',
      },
      tooltips: {
        enabled: true,
        intersect: false,
        mode: 'index',
        titleFontFamily: 'Fira Sans Condensed',
        bodyFontFamily: 'Fira Sans Condensed'
      },
      layout: {
        padding: {
          left: 15,
          right: 15,
          top: 15,
          bottom: 0
        }
      },
      animation: {
        duration: 0
      },
      scales: {
        xAxes: [{
          id: 'telemetrychart-x-axis-time',
          type: 'time',
          time: {
            parser: data => moment.utc(data),
            tooltipFormat: 'YYYY-MM-DD HH:mm',
            displayFormats: {
              millisecond: 'HH:mm:ss.SSS',
              second: 'HH:mm:ss',
              minute: 'HH:mm',
              hour: 'HH'
            },
          },
          display: true,
          gridLines: {
            display: true
          },
          ticks: {
            callback: function (value, index, values) {
              if (!values[index]) {
                return;
              }

              const currMoment = moment.utc(values[index].value);
              return currMoment.format('HH:mm');
            },
          },
        }, {
          id: 'telemetrychart-x-axis-date',
          type: 'time',
          gridLines: {
            display: false,
            drawBorder: false,
            drawTicks: false,
          },
          ticks: {
            callback: function (value, index, values) {
              if (!values[index]) {
                return;
              }

              const currMoment = moment.utc(values[index].value);
              const prevMoment = index > 0 && values[index - 1] ? moment.utc(values[index - 1].value) : null;
              const sameDay = prevMoment && currMoment.isSame(prevMoment, 'day');
              return sameDay ? '' : currMoment.format('MM/DD');
            },
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
            display: false,
          }
        }]
      }
    };
    return options;
  }

  addDataSource(): DataSource {
    const source = new DataSource(this.getMetricsSubject as Observer<any>, [
      new RoundCountersTransform(),
      new DisplayLabelTransform(), // This needs to be before groupByTransform
      new ColorTransform(),
      new GroupByTransform(),
      new GroupByTimeTransform(),
      new FieldSelectorTransform(),
      new LabelSelectorTransform(this.labelMap),
      new FieldValueTransform(this.fieldQueryMap, this.fieldValueMap), // This needs to be last to transform the query properly
    ]);
    source.debugMode = this.inDebugMode;
    this.dataSources.push(source);
    return source;
  }

  removeDataSource(index: number) {
    if (this.dataSources.length > 1) {
      this.dataSources.splice(index, 1);
    }
    // Check if
    this.getMetrics();
  }

  setTimeRange(timeRange: TimeRange) {
    // Pushing into next event loop so that angular's change detection
    // doesn't complain.
    setTimeout( () => {
      if (this.selectedTimeRange == null || !timeRange.isSame(this.selectedTimeRange)) {
        this.selectedTimeRange = timeRange;
        this.getMetrics();
      }
    }, 0);
  }

  /**
   * This API build MetricsPollingQuery and let user customize MetricsPollingQuery
   * @param source
   */
  buildMetricsPollingQuery(source: DataSource): MetricsPollingQuery {
    const query = MetricsUtility.timeSeriesQueryPolling(source.measurement, source.fields);
    if (source.measurement === 'Cluster' || source.measurement === 'SessionSummaryMetrics') {  // measurement can be SessionSummaryMetrics, FteCPSMetrics, Cluster
      query.query.function = Telemetry_queryMetricsQuerySpec_function.last; // VS-741 use median function to show DSC count
    }
    return query;
  }

  generateMetricsQuery(): TelemetryPollingMetricQueries {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    // Using variable so that this is not used inside updating timeSeriesQueryPolling
    // We may use an incorrect time instance if we do
    const timeRange = this.selectedTimeRange;
    this.dataSources.forEach( (source) => {
      const query = source.generateQuery(timeRange);
      if (query != null) {
         queryList.queries.push(query);
      }
    });
    this.graphTransforms.forEach( (t) => {
      t.transformQueries({
        queries: queryList
      });
    });
    return queryList;
  }

  areQueriesEqual(q1Input: TelemetryPollingMetricQueries, q2Input: TelemetryPollingMetricQueries): boolean {
    if (q1Input == null && q2Input == null) {
      return true;
    } else if (q1Input == null || q2Input == null) {
      return false;
    }
    if (q1Input.queries.length !== q2Input.queries.length) {
      return false;
    }
    const _  = Utility.getLodash();
    const queryUnequal = q1Input.queries.some( (q1, index) => {
      const q2 = q2Input.queries[index];
      const q1Query = Utility.getLodash().cloneDeep(q1.query);
      const q2Query = _.cloneDeep(q2.query);
      // Replace time fields so they aren't checked
      q1Query['start-time'] = null;
      q1Query['end-time'] = null;
      q2Query['start-time'] = null;
      q2Query['end-time'] = null;
      return !_.isEqual(q1Query, q2Query);
    });
    if (queryUnequal) {
      return false;
    }
    return true;
  }

  isTimeRangeSame(t1: TimeRange, t2: TimeRange) {
    if (t1 == null && t2 == null) {
      return true;
    } else if (t1 == null || t2 == null) {
      return false;
    }
    return t1.isSame(t2);
  }

  getMetrics() {
    const queryList: TelemetryPollingMetricQueries = this.generateMetricsQuery();
    if (queryList.queries.length === 0) {
      this.metricData = null;
      this.lastQuery = queryList;
      this.drawGraph();
      return;
    }
    // If it's the same query we don't need to generate a new query
    if (this.areQueriesEqual(this.lastQuery, queryList) && this.isTimeRangeSame(this.selectedTimeRange, this.lastTimeRange)) {
      this.drawGraph();
      return;
    }

    if (this.metricSubscription) {
      // discarding previous subscription incase it isn't completed yet.
      this.metricSubscription.unsubscribe();
      this.metricSubscription = null;
    }
    this.graphLoading = true;
    this.lastQuery = queryList;
    this.lastTimeRange = this.selectedTimeRange;
    this.metricsQueryObserver.next(queryList);
  }

  metricsQueryListener() {
    // Buffers requests for 200ms before issuing a
    // metrics request. This way we limit multiple requests going out when the component is being initalized
    const subscription = this.metricsQueryObserver.pipe(debounceTime(200)).subscribe(
      (queryList) => {
        this.metricSubscription = this.telemetryqueryService.pollMetrics('telemetryExplore-' + Utility.s4() + Utility.s4(), queryList).subscribe(
          (data: ITelemetry_queryMetricsQueryResponse) => {
            // If tenant is null, then it is the default response from the behavior subject, and not from the backend
            if (data.tenant != null) {
              this.metricData = data;
              this.drawGraph();
            }
          },
          (err) => {
            // Err is already logged to a toaster by the polling service.
            // Drawing graph so that the graph is reset
            // and we turn off the spinner
            this.drawGraph();
          }
        );
        this.subscriptions.push(this.metricSubscription);
      }
    );
    this.subscriptions.push(subscription);
  }

  resetGraph() {
    this.lineData = { datasets: [] };
    this.graphOptions = this.generateDefaultGraphOptions();
  }

  drawGraph() {
    if (this.metricData == null) {
      this.resetGraph();
      this.graphLoading = false;
      return;
    }

    if (!MetricsUtility.responseHasData(this.metricData)) {
      this.resetGraph();
      this.graphLoading = false;
      return;
    }

    // User can hide a dataset by clicking it's name on the legend
    // Create a map of which datasets are currently hidden, so that we can keep it.
    const hiddenDatasets = {};
    this.lineData.datasets.forEach( (dataset, index) => {
      if (this.chartContainer.chart && !this.chartContainer.chart.isDatasetVisible(index)) {  // VS-745 (saw some console errror. add a check)
        const key = sourceFieldKey(dataset.sourceID, dataset.sourceMeasurement, dataset.sourceField) + dataset.label;
        hiddenDatasets[key] = true;
      }
    });

    const results = this.metricData.results;

    this.graphTransforms.forEach( (t) => {
      t.transformMetricData({
        results: results
      });
    });

    // Start with a clean array so that we don't accidentally
    // have stale state transfer from the old options
    // All transforms should be relatively stateless
    const newGraphOptions: ChartOptions = this.generateDefaultGraphOptions();

    // Will be passed to graph transforms
    const allResultsDatasets = [];

    const resDataSets: ChartDataSets[] = [];
    results.forEach( (res, index) => {
      if (!MetricsUtility.resultHasData(res)) {
        // TODO: add only in legend
        return;
      }
      // Should be one to one with response length
      const source = this.dataSources[index];

      source.transformMetricData({ result: res });

      const singleResultDatasets: TransformDatasets = [];
      res.series.forEach( (s) => {
        source.fields.forEach( (field) => {
          const fieldIndex = MetricsUtility.findFieldIndex(s.columns, field);
          const data = MetricsUtility.transformToChartjsTimeSeries(s, field);
          const dataset: ChartDataSets = {
            data: data,
            pointRadius: 0,
            lineTension: 0,
            fill: false,
            sourceID: source.id,
            sourceField: field,
            spanGaps: true,
            sourceMeasurement: source.measurement,
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

    // Hide any lines that are already hidden
    resDataSets.forEach( (dataset, index) => {
      const key = sourceFieldKey(dataset.sourceID, dataset.sourceMeasurement, dataset.sourceField) + dataset.label;
      dataset.hidden = hiddenDatasets[key] != null ? hiddenDatasets[key] : false;
      dataset.steppedLine = (dataset.sourceMeasurement === 'Cluster') ? 'middle' : false; // VS-741 make cluster chart using stepped-line style
    });

    // modifications to legend after transforms
    if ((this.inEditMode && resDataSets.length > TelemetrychartComponent.MAX_LEGEND_EDIT_MODE) || (!this.inEditMode && resDataSets.length > TelemetrychartComponent.MAX_LEGEND_VIEW_MODE)) {
      newGraphOptions.legend.display = false;
    }

    this.graphOptions = newGraphOptions;

    this.lineData = {
      datasets: resDataSets
    };
    this.graphLoading = false;
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    if (this.metricSubscription != null) {
      this.metricSubscription.unsubscribe();
    }
  }

}
