import { Component, OnInit, ViewEncapsulation, Input, SimpleChanges, AfterViewInit } from '@angular/core';
import { Chart, ChartOptions, ChartPoint, ChartData } from 'chart.js';
import { PrettyDatePipe } from '../Pipes/PrettyDate.pipe';

export interface LineGraphStat {
  title: string;
  data: Array<number | null | undefined> | ChartPoint[];
  statColor: string;
  gradientStart: string;
  gradientStop: string;
  graphId: string;
  defaultValue: number;
  defaultDescription: string;
  // Called to format the value to display to the right of the graph
  valueFormatter?: (val: number) => string;
  hoverDescription: string;
  isPercentage: boolean;
}

@Component({
  selector: 'app-linegraph',
  templateUrl: './linegraph.component.html',
  styleUrls: ['./linegraph.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class LinegraphComponent implements OnInit, AfterViewInit {
  @Input() stats: LineGraphStat[] = [];
  @Input() statSpace: string = '100px';

  charts: Chart[] = [];
  statValues = {};
  canvasPadding = {
    right: 4,
    top: 10,
    bottom: 20, // height of the title label + 1 for spacing between line and graph
    left: 10
  };

  // Holds what data point is selected
  selectedIndex;

  chartsInitialized: boolean = false;

  public static percentFormatter(val: number): string {
    if (!val) {
      return '';
    }
    return val.toFixed(0) + '%';
  }


  constructor() { }

  ngOnInit() {
    // This component currently doesn't support stats having different number of entries
    // This is because on hover we use selected index for highlighting.
    // TODO: Move selectedIndex to selected time
    if (this.stats.length > 0) {
      const dataLength = this.stats[0].data.length;
      this.stats.some((stat) => {
        if (stat.data.length !== dataLength) {
          console.error('linegraph component does not support stats having different number of entries');
          return true;
        }
        return false;
      });
    }
    this.updateStatValues();
  }

  ngOnChange(changes: SimpleChanges) {
    if (this.chartsInitialized &&
      changes.stats.currentValue.length !== changes.stats.previousValue.length) {
      console.error('Linegraph component does not currently support changing number of graphs dynamically');
    }
    this.setupCharts();
  }

  updateStatValues() {
    this.stats.forEach((stat, index) => {
      let valueString = '';
      let value = stat.defaultValue;
      let desc = stat.defaultDescription;
      if (this.selectedIndex != null) {
        const chartPoint: ChartPoint = <ChartPoint>this.charts[index].data.datasets[0].data[this.selectedIndex];
        if (chartPoint != null) {
          value = chartPoint.y as number;
          desc = stat.hoverDescription;
        }
      }
      if (stat.valueFormatter) {
        valueString = stat.valueFormatter(value);
      } else {
        // By default we round to one decimal
        if (value != null) {
          valueString = value.toFixed(1);
          if (valueString.endsWith('.0')) {
            // Remove ending .0
            valueString = valueString.slice(0, valueString.length - 2);
          }
        }
      }
      this.statValues[stat.graphId] = {
        value: valueString,
        description: desc
      };
    });
  }

  resetSelectedPoint() {
    this.selectedIndex = null;
    this.updatePointStyle();
    this.updateStatValues();
  }

  /**
   * Draws the highlighting of the selected points
   */
  updatePointStyle() {
    const helper = (dataset) => {
      const pointBackgroundColor = [];

      for (let index = 0; index < dataset.data.length; index++) {
        if (index === this.selectedIndex) {
          pointBackgroundColor.push('#676763');
        } else {
          pointBackgroundColor.push('rgba(0, 0, 0, 0)');
        }
      }
      dataset.pointBackgroundColor = pointBackgroundColor;
    };

    this.charts.forEach((chart) => {
      helper(chart.data.datasets[0]);
      chart.update();
    });
  }

  ngAfterViewInit() {
    // Waiting till next cycle
    // so that heights are properly set
    setTimeout(() => {
      this.setupCharts();
    }, 0);
  }

  /**
   * Initializes the charts, or updates them if the chart is already
   * initialized
   */
  setupCharts() {
    if (this.chartsInitialized) {
      this.stats.forEach((stat, index) => {
        const chart = this.charts[index];
        chart.data.datasets[0].data = stat.data;
        chart.update();
        // We reset the selected index in case it is now out of bounds
        this.selectedIndex = null;
        this.updatePointStyle();
        this.updateStatValues();
      });
    } else {
      this.chartsInitialized = true;
      this.stats.forEach((stat) => {
        const chart = this.genChart(stat);
        this.charts.push(chart);
      });
    }
  }

  /**
   * Generates the Chart object and draws it to the canvas
   * with the passed in id
   * @param stat
   */
  genChart(stat: LineGraphStat) {
    const id = stat.graphId;
    const color = stat.statColor;
    const gradientStart = stat.gradientStart;
    const gradientStop = stat.gradientStop;
    const data = stat.data;

    const canvas: any = document.getElementById(id);
    const canvasContainer: any = document.getElementById(id + '-container');
    const canvasHeight = canvasContainer.clientHeight;
    const ctx = canvas.getContext('2d');
    const gradientFill = ctx.createLinearGradient(0, 0, 0, canvasHeight - this.canvasPadding.bottom - 1);
    gradientFill.addColorStop(0, gradientStart);
    gradientFill.addColorStop(1, gradientStop);

    const dataLinegraph: ChartData = {
      datasets: [
        {
          data: data,
          backgroundColor: gradientFill,
          borderColor: color,
          borderWidth: 2,
          pointBorderColor: 'rgba(0, 0, 0, 0)',
          pointBackgroundColor: 'rgba(0, 0, 0, 0)',
        },
      ]
    };

    return new Chart(canvas, {
      type: 'line',
      data: dataLinegraph,
      options: this.generateOptions(id + 'Tooltip', stat.isPercentage),
    });
  }

  /**
   * Generates line graph options used for the charts.
   *
   * @param chartTooltipId Id to be used when attaching an element to the DOM for the tooltip
   * @param isPercentage Whether the stat is of a percentage or not.
   *                     Used to determine whether to auto scale
   *                     or to use 0-100 scale.
   */
  generateOptions(chartTooltipId, isPercentage): ChartOptions {
    const ret: ChartOptions = {
      maintainAspectRatio: false,
      layout: {
        padding: this.canvasPadding
      },
      legend: {
        display: false,
      },
      elements: {
      },
      tooltips: {
        enabled: false,
        mode: 'index',
        intersect: false,
        position: 'nearest',
        custom: this.createCustomTooltip(chartTooltipId)
      },
      animation: {
        duration: 0
      },
      title: {
        display: false
      },
      scales: {
        xAxes: [{
          type: 'time',
          display: false,
          gridLines: {
            display: false
          },
          scaleLabel: {
            display: false
          },
          ticks: {
            display: false,
          },
        }],
        yAxes: [{
          type: 'linear',
          gridLines: {
            display: false
          },
          display: false,
          scaleLabel: {
            display: false
          },
          ticks: {
            display: false,
          }
        }]
      },
      hover: {
        mode: 'index',
        intersect: false,
        onHover: (event, item: any) => {
          if (item.length) {
            this.selectedIndex = item[0]._index;
            this.updatePointStyle();
            this.updateStatValues();
          }
        },
      },
      plugins: {
        datalabels: {
          display: false,
        },
      }
    };
    if (isPercentage) {
      ret.scales.yAxes[0].ticks.min = 0;
      ret.scales.yAxes[0].ticks.max = 100;
    }
    return ret;
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
        const titleLines = tooltip.title || [];
        const prettyDate = new PrettyDatePipe('en-US');

        titleLines.forEach(function(title) {
          tooltipEl.innerHTML = prettyDate.transform(title);
        });
      }
      const positionY = chartThis._chart.canvas.offsetTop;
      const positionX = chartThis._chart.canvas.offsetLeft;
      // Display, position, and set styles for font
      tooltipEl.style.opacity = '1';
      tooltipEl.style.left = positionX + tooltip.caretX + 'px';
      tooltipEl.style.top = positionY + tooltip.caretY + 'px';
      tooltipEl.style.fontFamily = tooltip._bodyFontFamily;
      tooltipEl.style.fontSize = tooltip.bodyFontSize + 'px';
      tooltipEl.style.fontStyle = tooltip._bodyFontStyle;
      tooltipEl.style.padding = tooltip.yPadding + 'px ' + tooltip.xPadding + 'px';
    };
  }

}
