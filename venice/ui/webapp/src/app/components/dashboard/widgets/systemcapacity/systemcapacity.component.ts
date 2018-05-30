import { Component, OnChanges, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-dsbdsystemcapacitywidget',
  templateUrl: './systemcapacity.component.html',
  styleUrls: ['./systemcapacity.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class SystemcapacitywidgetComponent implements OnInit, OnDestroy, OnChanges {
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
  data: any;

  venice_node_count = 5;
  title = 'Venice System Capacity';
  last_update: string = 'Last Updated: ' + Utility.getPastDate(3).toLocaleDateString();
  icon: Icon = {
    margin: {
      top: '8px',
      left: '10px'
    },
    svgIcon: 'venice'
  };
  background_img: any = {
    url: 'assets/images/dashboard/venice.svg'
  };
  menuItems: any[] = [
    { text: 'Toggle Trend Line' },
    { text: 'Trouble shooting' }
  ];
  content: 'Venice System Capacity';

  constructor() {
  }

  ngOnInit() {
    // if (!this.item.id) {
    this.id = 'systemcapacity-' + Utility.s4();
    // }
    this.setGraphOptions();
    this._background_img = this.setBackgroundImg();
  }

  menuselect(obj) {
    if (obj.menu.text === 'Toggle Trend Line') {
      this.menuItems[0] = { text: 'Toggle Bar Chart' };
    } else if (obj.menu.text === 'Toggle Bar Chart') {
      this.menuItems[0] = { text: 'Toggle Trend Line' };
    }
    console.log('workload menuselect()', obj);
  }

  setBackgroundImg() {
    const styles = {
      'background-image': 'url(' + this.background_img.url + ')',
    };
    return styles;
  }

  /**
   * Compute whether to show System-Capacity snapshop Chart tooltip when mouse is over the chart
   * @param tooltipItem
   * @param data
   */
  isToShowSystemCapacityBarChartTooltip(tooltipItem, data): boolean {

    if (tooltipItem.datasetIndex === 1) {
      return false;
    }
    return true;
  }

  ngOnDestroy() {
  }

  ngOnChanges() {
  }

  itemClick($event, datsetType) {
    const obj = {
      event: $event,
      dataset: datsetType
    };
    console.log('systemcapacity click', obj);
  }

  setGraphOptions() {
    const dataList = Utility.getRandomIntList(4, 10, 50);
    const labels = ['Storage', 'CPU', 'Memory', 'Nework'];
    const maxNum = 50;
    const dataMaxList = [maxNum, maxNum, maxNum, maxNum];

    this.layout = {
      annotations: [{
        x: 'Storage',
        xanchor: 'center',
        y: 120,
        yanchor: 'top',
        showarrow: false,
        text: 'Hello'
      }, {
        x: 'Storage',
        xanchor: 'center',
        y: 22,
        yanchor: 'top',
        text: 'Y axis label',
        showarrow: false
      }]
    };

    this.data = {
      labels: labels,
      datasets: [
        {
          backgroundColor: ['#E1D4F0', '#CBB2EE', '#E1D4F0', '#CBB2EE'],
          borderColor: ['#CBB2EE', '#B594E1', '#CBB2EE', '#B594E1'],
          borderWidth: 1,
          data: dataList, // [15, 25, 27, 34],
          datalabels: {
            align: 'end',
            color: function(context) {
              return context.dataset.borderColor;
            },
            font: {
              size: 12,
              weight: 'bold'
            },
            formatter: function(value, context) {
              return (context.dataIndex % 2 === 0) ? value + ' GB' : value;
            },
            offset: -20 // put he lable inside the red bar areas.
          }
        },
        {
          backgroundColor: '#eee',
          borderColor: '#aaa',
          borderWidth: 1,
          data: dataMaxList, // [50, 50, 50, 50],
          datalabels: {
            align: 'start',
            color: '#666',
            font: {
              size: 12,
              weight: 500
            },
            formatter: function(value, context) {
              const ds0 = context.chart.data.datasets[0];
              const idx = context.dataIndex;
              const v0 = ds0.data[idx];

              return Math.round(100 * v0 / value) + '%';
            },
            offset: 0
          }
        }
      ]
    },

      this.options = {
        maintainAspectRatio: true, // important to set chart height
        responsive: false,
        legend: false,
        tooltips: {
          enabled: true,
          filter: (tooltipItem, data) => {
            return this.isToShowSystemCapacityBarChartTooltip(tooltipItem, data);
          },
          callbacks: {
            title: function(tooltipItem, data) {
              return (tooltipItem.length > 0) ? data['labels'][tooltipItem[0]['index']] : null;
            },
            label: function(tooltipItem, data) {
              return (tooltipItem) ? data['datasets'][tooltipItem.datasetIndex].data[tooltipItem.index] : null;
            },


          }
        },
        layout: {
          padding: {
            left: 0,
            right: 0,
            top: 0,
            bottom: 0
          }
        },
        scales: {
          xAxes: [{
            stacked: true,
            gridLines: false,
            ticks: {
              padding: 5,
              fontSize: 12 // x-axis label (storage, CPU) font size
            },
            categoryPercentage: 1.0, // distance between two bars
          }],
          yAxes: [{
            display: false,
            ticks: {
              min: 0,
              max: maxNum + 10
            }
          }]
        },
        plugins: {
          datalabels: {
            anchor: 'end',
            offset: 2,
            font: {
              family: 'Fira Sans'
            }
          }
        }
      },

      this.trendline = {
        data: {
          labels: ['W1', 'W2', 'W3', 'W4', 'W5', 'W6', 'W7'],
          datasets: [
            {
              lineTension: 0,
              label: 'Storage',
              data: Utility.getRandomIntList(7, 23, 40), // 25, 29, 30, 23, 26, 36, 30],
              fill: false,
              borderColor: '#4bc0c0'
            },
            {
              lineTension: 0,
              label: 'CPU',
              data: Utility.getRandomIntList(7, 18, 40), // [28, 38, 40, 19, 36, 27, 40],
              fill: false,
              borderColor: '#565656'
            },
            {
              lineTension: 0,
              label: 'Memory',
              data: Utility.getRandomIntList(7, 32, 60), // [35, 49, 50, 41, 56, 55, 40],
              fill: false,
              borderColor: '#97b8df'
            },
            {
              lineTension: 0,
              label: 'Network',
              data: Utility.getRandomIntList(7, 20, 45), // [25, 49, 23, 43, 36, 35, 33],
              fill: false,
              borderColor: '#e57553'
            },
          ]
        },
        options: {
          title: {
            display: false
          },
          elements: {
            point: {
              radius: 2,
              hitRadius: 10,
              hoverRadius: 5,
            }
          },
          plugins: {
            datalabels: {
              display: false
            }
          },
          tooltips: {
            enabled: true,
            mode: 'dataset',
            position: 'nearest',
            intersect: false,
            // yAlign:'top'
          },
          legend: {
            display: true,
            position: 'right',
            labels: {
              usePointStyle: true,
              fontSize: 9
            }
          },
          responsive: true,
          scales: {
            xAxes: [{
              display: true,
              gridLines: {
                display: true,
                drawBorder: false
              }

            }],
            yAxes: [{
              display: true,
              gridLines: {
                display: true,
                drawBorder: false
              },
              ticks: { min: 15 }
            }]
          }
        }
      };


  }
}
