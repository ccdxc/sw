import { Component, Input, OnChanges, OnDestroy, OnInit, ViewChild, ViewEncapsulation, AfterViewInit } from '@angular/core';
import { Animations } from '@app/animations';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { Utility } from '@app/common/Utility';
import { CardStates, StatArrowDirection } from '@app/components/shared/basecard/basecard.component';
import { LinegraphComponent, LineGraphStat } from '@app/components/shared/linegraph/linegraph.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { IMetrics_queryQueryResult } from '@sdk/v1/models/metrics_query';
import { Chart, ChartData } from 'chart.js';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ClusterNode } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';
import { Subscription } from 'rxjs';
import { ControllerService } from '@app/services/controller.service';
import { FlipState, FlipComponent } from '@app/components/shared/flip/flip.component';

interface BarGraphStat {
  percent: number;
  hoverText?: string;
  arrowDirection?: StatArrowDirection;
  statColor: string;
}

interface GraphStat {
  title: string;
  lineGraphStat: LineGraphStat;
  barGraphStat: BarGraphStat;
  fieldName: string;
  barChart: Chart;
  id: string;
}

interface NetworkGraphStat {
  lineGraphStat: LineGraphStat;
  currentValue: {
    arrowDirection: StatArrowDirection,
    value: string,
    description: string,
    statColor: string
  };
  fieldNames: string[];
}

@Component({
  selector: 'app-dsbdsystemcapacitywidget',
  templateUrl: './systemcapacity.component.html',
  styleUrls: ['./systemcapacity.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class SystemcapacitywidgetComponent implements OnInit, AfterViewInit, OnDestroy, OnChanges {
  @ViewChild('lineGraph') lineGraphComponent: LinegraphComponent;
  viewInitialized: boolean = false;
  _background_img: any;
  dataset: any;
  layout_use: any;
  options_use: any;
  color: String;
  style_class: String;
  id: string;

  trendline: any;
  options: any;
  layout: any;
  data: ChartData;

  title = 'Cluster';
  last_update: string = 'Last Updated: ' + Utility.getPastDate(3).toLocaleDateString();
  icon: Icon = {
    margin: {
      top: '8px',
      left: '10px'
    },
    svgIcon: 'cluster'
  };
  background_img: any = {
    url: 'assets/images/dashboard/venice.svg'
  };
  menuItems = [
    {
      text: 'Flip card', onClick: () => {
        this.toggleFlip();
      }
    },
  ];
  flipState: FlipState = FlipState.front;

  cpuLineGraphStat: LineGraphStat = {
    title: 'CPU',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'systemcapacity-cpu',
    defaultValue: null,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: true,
    valueFormatter: LinegraphComponent.percentFormatter
  };
  memLineGraphStat: LineGraphStat = {
    title: 'MEMORY',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'systemcapacity-memory',
    defaultValue: null,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: true,
    valueFormatter: LinegraphComponent.percentFormatter
  };
  storageLineGraphStat: LineGraphStat = {
    title: 'STORAGE',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'systemcapacity-storageStat',
    defaultValue: null,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: true,
    valueFormatter: LinegraphComponent.percentFormatter
  };
  networkLineGraphStat: LineGraphStat = {
    title: 'NETWORK',
    data: [],
    statColor: '#b592e3',
    gradientStart: 'rgba(181,146, 227,1)',
    gradientStop: 'rgba(181,146, 227,0)',
    graphId: 'systemcapacity-network',
    defaultValue: null,
    defaultDescription: 'Avg',
    hoverDescription: '',
    isPercentage: false,
    valueFormatter: (val) => {
      if (val) {
        return Utility.formatBytes(val, 2, 3);
      }
      return '';
    }
  };

  cpuGraphStat: GraphStat = {
    title: 'CPU',
    lineGraphStat: this.cpuLineGraphStat,
    barGraphStat: {
      percent: 0,
      statColor: '#b592e3',
    },
    fieldName: 'mean_CPUUsedPercent',
    barChart: null,
    id: 'systemcapacity-bargraph-cpu'
  };

  memGraphStat: GraphStat = {
    title: 'MEMORY',
    lineGraphStat: this.memLineGraphStat,
    barGraphStat: {
      percent: 0,
      statColor: '#b592e3',
    },
    fieldName: 'mean_MemUsedPercent',
    barChart: null,
    id: 'systemcapacity-bargraph-memory'
  };

  storageGraphStat: GraphStat = {
    title: 'STORAGE',
    lineGraphStat: this.storageLineGraphStat,
    barGraphStat: {
      percent: 0,
      statColor: '#b592e3',
    },
    fieldName: 'mean_DiskUsedPercent',
    barChart: null,
    id: 'systemcapacity-bargraph-storage'
  };

  networkGraphStat: NetworkGraphStat = {
    lineGraphStat: this.networkLineGraphStat,
    fieldNames: ['mean_InterfaceRxBytes', 'mean_InterfaceTxBytes'],
    currentValue: {
      arrowDirection: StatArrowDirection.HIDDEN,
      value: '10.6 Mb',
      description: '',
      statColor: '#b592e3',
    }
  };

  barGraphDrawn: boolean = false;

  cpuChart: Chart;
  memChart: Chart;
  storageChart: Chart;

  themeColor: string = '#b592e3';
  statColor: string = '#77a746';
  @Input() currentData: IMetrics_queryQueryResult;
  @Input() prevData: IMetrics_queryQueryResult;
  @Input() timeSeriesData: IMetrics_queryQueryResult;
  @Input() avgDayData: IMetrics_queryQueryResult;

  @Input() lastUpdateTime: string;
  @Input() timeRange: string;
  // When set to true, card contents will fade into view
  @Input() cardState: CardStates = CardStates.READY;

  linegraphStats: LineGraphStat[] = [
    this.cpuLineGraphStat,
    this.memLineGraphStat,
    this.storageLineGraphStat,
    this.networkLineGraphStat
  ];

  graphStats: GraphStat[] = [
    this.cpuGraphStat,
    this.memGraphStat,
    this.storageGraphStat,
  ];

  cardStates = CardStates;

  nodeEventUtility: HttpEventUtility<ClusterNode>;
  nodes: ReadonlyArray<ClusterNode> = [];

  subscriptions: Subscription[] = [];

  constructor(protected controllerService: ControllerService,
    protected clusterService: ClusterService) {
  }

  toggleFlip() {
    this.flipState = FlipComponent.toggleState(this.flipState);
  }

  ngOnInit() {
    this.getNodes();
    this._background_img = this.setBackgroundImg();
  }

  setBackgroundImg() {
    const styles = {
      'background-image': 'url(' + this.background_img.url + ')',
    };
    return styles;
  }

  getNodes() {
    this.nodeEventUtility = new HttpEventUtility<ClusterNode>(ClusterNode);
    this.nodes = this.nodeEventUtility.array;
    const subscription = this.clusterService.WatchNode().subscribe(
      response => {
        this.nodeEventUtility.processEvents(response);
      },
      this.controllerService.restErrorHandler('Failed to get Node info')
    );
    this.subscriptions.push(subscription);
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  ngOnChanges() {
    this.setupData();
  }

  ngAfterViewInit() {
    // We wait for the view to be initialized as we need the linegraph charts
    // to be ready in the dom.
    this.viewInitialized = true;
    this.setupData();
  }

  setupData() {
    if (this.cardState === CardStates.READY && this.viewInitialized) {
      this.graphStats.forEach((entry) => {
        const lineGraph = entry.lineGraphStat;
        const barGraph = entry.barGraphStat;
        const fieldName = entry.fieldName;

        // TimeSeries data
        if (MetricsUtility.resultHasData(this.timeSeriesData)) {
          const index = this.timeSeriesData.series[0].columns.indexOf(fieldName);
          const data = Utility.transformToChartjsTimeSeries(this.timeSeriesData.series[0].values, 0, index);
          lineGraph.data = data;
        }

        // day average data
        if (MetricsUtility.resultHasData(this.avgDayData)) {
          const index = this.avgDayData.series[0].columns.indexOf(fieldName);
          lineGraph.defaultValue = Math.round(this.avgDayData.series[0].values[0][index]);
        }

        // currentData
        if (MetricsUtility.resultHasData(this.currentData)) {
          let index = this.currentData.series[0].columns.indexOf(fieldName);
          barGraph.percent = this.currentData.series[0].values[0][index];
          index = this.prevData.series[0].columns.indexOf(fieldName);
          const prev = this.prevData.series[0].values[0][index];
          barGraph.arrowDirection = MetricsUtility.getStatArrowDirection(prev, barGraph.percent);
        }
      });

      // Network handled separately as we have to add rx and tx
      // TimeSeries data
      const fieldName1 = this.networkGraphStat.fieldNames[0];
      const fieldName2 = this.networkGraphStat.fieldNames[1];
      if (MetricsUtility.resultHasData(this.timeSeriesData)) {
        const index1 = this.timeSeriesData.series[0].columns.indexOf(fieldName1);
        const data1 = Utility.transformToChartjsTimeSeries(this.timeSeriesData.series[0].values, 0, index1);
        const index2 = this.timeSeriesData.series[0].columns.indexOf(fieldName2);
        const data2 = Utility.transformToChartjsTimeSeries(this.timeSeriesData.series[0].values, 0, index2);

        for (let index = 0; index < data1.length; index++) {
          data1[index].y = data1[index].y + data2[index].y;
        }
        this.networkGraphStat.lineGraphStat.data = data1;

        const lastItem = data1[data1.length - 1];
        const secondToLastItem = data1[data1.length - 2];
        const thirdToLastItem = data1[data1.length - 3];
        const currentRate = lastItem.y - secondToLastItem.y;
        const previousRate = secondToLastItem.y - thirdToLastItem.y;

        this.networkGraphStat.currentValue.value = Utility.formatBytes(currentRate, 2, 3);
        this.networkGraphStat.currentValue.arrowDirection = MetricsUtility.getStatArrowDirection(previousRate, currentRate);
      }

      // day average data
      if (MetricsUtility.resultHasData(this.avgDayData)) {
        const index1 = this.avgDayData.series[0].columns.indexOf(fieldName1);
        const index2 = this.avgDayData.series[0].columns.indexOf(fieldName2);

        const data1 = Math.round(this.avgDayData.series[0].values[0][index1]);
        const data2 = Math.round(this.avgDayData.series[0].values[0][index2]);

        this.networkGraphStat.lineGraphStat.defaultValue = data1 + data2;
      }

      if (this.barGraphDrawn) {
        this.graphStats.forEach((entry) => {
          this.updateBarGraph(entry.barChart, entry.barGraphStat);
        });
        // Manually calling setup charts to redraw as default
        // change detection will not trigger when the data changes
        this.lineGraphComponent.setupCharts();
      } else {
        this.barGraphDrawn = true;
        // In case we just switched from loading state to ready state,
        // we need to wait for dom to render the canvas
        setTimeout(() => {
          this.graphStats.forEach((entry) => {
            entry.barChart = this.setBarGraphOptions(entry.barGraphStat, entry.id);
          });
          // change detection will not trigger when the data changes
          this.lineGraphComponent.setupCharts();
        }, 0);
      }
    }
  }

  updateBarGraph(chart: Chart, barGraphStat: BarGraphStat) {
    if (chart == null || barGraphStat == null) {
      return;
    }
    const percent = barGraphStat.percent;
    chart.data.datasets[0].data = [percent];
    chart.data.datasets[1].data = [100 - percent];
    chart.update();
  }

  setBarGraphOptions(graphData: BarGraphStat, id: string): Chart {
    const dataList = [graphData.percent];
    const labels = [''];
    // compliment of the percentages
    // in order to make outer bar
    const dataMaxList = [100 - graphData.percent];

    this.layout = {};

    this.data = {
      labels: labels,
      datasets: [
        {
          backgroundColor: ['#CBB2EE'],

          borderColor: ['#CBB2EE'],
          hoverBackgroundColor: ['#b592e3'],
          borderWidth: 1,
          data: dataList,
        },
        {
          backgroundColor: '#eee',
          borderColor: '#aaa',
          hoverBackgroundColor: '#d6d6d6',
          borderWidth: 1,
          data: dataMaxList,
        }
      ]
    };

    this.options = {
      maintainAspectRatio: true, // important to set chart height
      responsive: false,
      legend: false,
      tooltips: {
        enabled: false,
        mode: 'index',
        intersect: false,
        position: 'nearest',
        custom: this.createCustomTooltip(id + '-tooltip')
      },
      layout: {
        padding: {
          left: 2,
          right: 2,
          top: 2,
          bottom: 2
        }
      },
      scales: {
        xAxes: [{
          stacked: true,
          gridLines: false,
          display: false,
          ticks: {
            display: false
          },
          barPercentage: 1
        }],
        yAxes: [{
          barThickness: 20,
          stacked: true,
          gridLines: false,
          display: false,
        }]
      },
      hover: {
        mode: 'index',
        intersect: false,
      },
      plugins: {
        datalabels: {
          display: false,
        },
      }
    };

    const canvas: any = document.getElementById(id);

    return new Chart(canvas, {
      type: 'horizontalBar',
      data: this.data,
      options: this.options,
    });

  }

  /**
   * Returns a function to be passed to chartjs for creating tooltips.
   * This function creates tooltips on the DOM instead of the canvas so that
   * it won't be clipped by the size of the canvas.
   */
  createCustomTooltip(chartTooltipId) {
    return function(tooltip) {
      // This function will be called in the context of the chart
      const chartThis: any = this;
      // Tooltip Element
      let tooltipEl = document.getElementById(chartTooltipId);
      if (!tooltipEl) {
        tooltipEl = document.createElement('div');
        tooltipEl.id = chartTooltipId;
        tooltipEl.classList.add('global-chartjs-tooltip');
        chartThis._chart.canvas.parentNode.appendChild(tooltipEl);
      }
      // Hide if no tooltip
      if (tooltip.opacity === 0) {
        tooltipEl.style.opacity = '0';
        return;
      }
      // Set caret Position
      tooltipEl.classList.remove('above', 'below', 'no-transform');
      if (tooltip.yAlign) {
        tooltipEl.classList.add(tooltip.yAlign);
      } else {
        tooltipEl.classList.add('no-transform');
      }
      // Set Text
      if (tooltip.body) {
        tooltipEl.innerHTML = tooltip.dataPoints[0].xLabel.toFixed(1) + ' %';
      }
      const positionY = chartThis._chart.canvas.offsetTop;
      const positionX = chartThis._chart.canvas.offsetLeft;
      // Display, position, and set styles for font
      tooltipEl.style.opacity = '1';
      tooltipEl.style.left = positionX + 130 + 'px';
      tooltipEl.style.top = positionY + tooltip.caretY + 'px';
      tooltipEl.style.fontFamily = tooltip._bodyFontFamily;
      tooltipEl.style.fontSize = tooltip.bodyFontSize + 'px';
      tooltipEl.style.fontStyle = tooltip._bodyFontStyle;
      tooltipEl.style.padding = tooltip.yPadding + 'px ' + tooltip.xPadding + 'px';
    };
  }
}
