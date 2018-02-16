import { Component, OnDestroy, OnInit, ViewEncapsulation, ViewChild, ElementRef } from '@angular/core';
import { FormsModule } from '@angular/forms';

import { ControllerService } from '../../services/controller.service';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { Logintypes } from '../../enum/logintypes.enum';
import { Utility } from '../../common/Utility';
import { BaseComponent } from '../base/base.component';

/**
 * This is Dashboard Component for VeniceUI
 * html has multiple ng-template registered.  This component's widgets will map to ng-template.
 *
 * We use gridster widget to enable grid layout and enable grid-item drag-and-drop
 * DashboardComponent.gridsterOptions defines the grid (2x6) matrix. Each cell is 310px height. css .dsbd-main-gridster-content defines the gridster height as 650.
 * Thus, we must be careful to create widget which will be hosted in a grid-item in 300px height.
 *
 * We use canvas API to draw extra texts in charts.  We are using chart.js charts as it offers better animation look & feel.  See "Naple and Workload" configurations.
 */
@Component({
  selector: 'app-dashboard',
  templateUrl: './dashboard.component.html',
  styleUrls: ['./dashboard.component.scss']
})
export class DashboardComponent extends BaseComponent implements OnInit, OnDestroy {

  gridsterOptions: any = {
    lanes: 6,
    lines: 2,
    direction: 'vertical',
    dragAndDrop: true,
    shrink: true,
    responsiveView: true,
    cellHeight: 310
  };
  title = 'Venice UI Dashboard';

  widgets: any;

  constructor(protected _controllerService: ControllerService) {
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
      this.getDashboardData();
    }
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

  protected onSelectData(event, chartId) {
    this.info('Dashboard.policyHeath.chart.dataSelect ', event, chartId);
  }

  private _build_SoftwareVersion(): any {
    return {
      id: 'software_version',
      x: 2, y: 0, w: 2, h: 1,
      title: 'Software Version',
      content: 'Version 1.3.0'
    };
  }
  private _build_SystemCapacity(): any {
    return {
      x: 0, y: 0, w: 2, h: 1,
      id: 'system_capacity',
      title: 'Venice System Capacity',
      content: 'Venice System Capacity',
      data: {
        labels: ['Storage', 'CPU', 'Memory', 'Nework'],
        datasets: [{
          label: 'Usage',
          backgroundColor: '#ccb0ef',
          data: [
            30,
            50,
            10,
            20
          ]
        }, {
          label: 'Capacity',
          backgroundColor: '#F1ECE8',
          borderColor: '#8c8779',
          data: [
            70,
            50,
            90,
            80
          ]
        }]

      },
      options: {
        title: {
          display: false,
          text: 'Capacity'
        },
        tooltips: {
          mode: 'index',
          intersect: false
        },
        legend: {
          display: false
        },

        responsive: true,
        scales: {
          xAxes: [{
            gridLines: {
              display: false,
              drawBorder: false
            },
            stacked: true,
          }],
          yAxes: [{
            display: false,
            stacked: true,
            gridLines: {
              display: false,
              drawBorder: false
            }
          }]
        }, plugins: {
          datalabels: {
            color: '#333',
            textAlign: 'center',
            display: function (context) {
              return context.datasetIndex === 1;
            },
            formatter: function (value, context) {
              if (context.dataIndex === 0) {
                return Math.round(100 - value) + '%\n 15 GB';
              } else if (context.dataIndex === 1) {
                return Math.round(100 - value) + '%';
              } else if (context.dataIndex === 2) {
                return Math.round(100 - value) + '%\n 50 GB';
              } else if (context.dataIndex === 3) {
                return Math.round(100 - value) + '%';
              } else {
                return value;
              }
            },
            font: {
              weight: 'bold',
              family: 'Fira San'
            }

          }
        }
      }
    };
  }
  private _build_PoliciesHealth(): any {
    return {
      x: 4, y: 0, w: 2, h: 1,
      id: 'policy_health',
      title: 'Policies Health',
      content: 'Policies Health',
      data: {
        labels: ['Good', 'Bad'],
        datasets: [
          {
            data: [92, 8],
            backgroundColor: [
              '#FF6384',
              '#36A2EB'
            ],
            hoverBackgroundColor: [
              '#FF6384',
              '#36A2EB'
            ]
          }]
      },
      options: {
        circumference: 2 * Math.PI,
        rotation: 1.1 * Math.PI,
        plugins: {
          datalabels: {
            backgroundColor: function (context) {
              return context.dataset.backgroundColor;
            },
            borderColor: 'white',
            borderRadius: 25,
            borderWidth: 2,
            color: 'white',
            display: function (context) {
              const dataset = context.dataset;
              const count = dataset.data.length;
              const value = dataset.data[context.dataIndex];
              return value > count * 1.5;
            },
            font: {
              weight: 'bold',
              family: 'Fira San'
            },
            formatter: Math.round
          }
        },
        animation: {
          onComplete: function () {
            //
            // see. dashboard.component.html <ng-template #dashboardPolicyHealth . There is a <canvas id="policy_health_text"
            // we employ the chart.js onComplete() to draw "8%" to the center of donut chart
            //
            const total = 'GOOD';
            const $ = Utility.getJQuery();
            const element = $('#policy_health_text').get(0);
            if (element) {
              const textCtx = element.getContext('2d');
              textCtx.textAlign = 'center';
              textCtx.textBaseline = 'middle';
              textCtx.font = '14px Fira San';
              textCtx.fillText(total, 150, 125);
            }
          }
        }
      },

    };
  }

  private _build_Workloads(): any {
    return {
      x: 0, y: 1, w: 2, h: 1,
      id: 'workloads',
      title: 'Workloads',
      content: 'Workloads',
      data: {
        labels: ['Week1', 'Week2'],
        datasets: [{
          label: 'Increase %',
          backgroundColor: '#4dc9f6',
          data: [
            12,
            14
          ]
        }, {
          label: 'Capacity %',
          backgroundColor: '#8549ba',
          data: [
            88,
            86
          ]
        }]

      },
      options: {
        title: {
          display: false,
          text: 'Workloads'
        },
        tooltips: {
          mode: 'index',
          intersect: false
        },
        legend: {
          display: false
        },

        responsive: true,
        scales: {
          xAxes: [{
            gridLines: {
              display: false,
              drawBorder: false
            },
            stacked: true,
          }],
          yAxes: [{
            display: false,
            stacked: true,
            gridLines: {
              display: false,
              drawBorder: false
            }
          }]
        }, plugins: {
          datalabels: {
            color: 'white',
            textAlign: 'center',
            display: function (context) {
              return context.datasetIndex === 1;
            },
            formatter: function (value, context) {
              return Math.round(100 - value) + '%\n INCREASE';
            },
            font: {
              weight: 'bold',
              family: 'Fira San'
            }

          }
        }
      }
    };
  }
  private _buildNaples(): any {
    const self = this;
    const obj = this._getNaplesNumbers();
    const badLabel = obj['badLabel'];
    const goodLabel = obj['goodLabel'];
    const badNum: Number = obj['badNum'];
    const goodNum: Number = obj['goodNum'];
    const numPercent = obj['percent'];
    return {
      x: 2, y: 1, w: 4, h: 1,
      id: 'naples',
      title: 'Naples',
      content: 'Naples',
      data: {
        labels: [goodLabel, badLabel],
        datasets: [
          {
            data: [goodNum, badNum],
            backgroundColor: [
              '#88b358',
              '#eeeeee'
            ]
          },
          {
            data: [badNum, goodNum],
            backgroundColor: [
              '#e57553',
              '#eeeeee'
            ]
          }
        ]
      },
      options: {
        circumference: 1.8 * Math.PI,
        rotation: -1.42 * Math.PI,
        cutoutPercentage: 70,
        plugins: {
          datalabels: {
            display: false
          }
        },
        animation: {
          onComplete: function () {
            //
            // see. dashboard.component.html <ng-template #dashboardPolicyHealth . There is a <canvas id="policy_health_text"
            // we employ the chart.js onComplete() to draw "8%" to the center of donut chart
            //
            const mainLabel = numPercent;
            const $ = Utility.getJQuery();
            const element = $('#naples_text').get(0);
            if (element) {
              const textCtx = element.getContext('2d');
              textCtx.textAlign = 'center';
              textCtx.textBaseline = 'middle';
              textCtx.font = '24px Fira San';
              textCtx.fillStyle = '#676763';
              textCtx.fillText(mainLabel, 180, 70);
              textCtx.font = '10px Fira San';
              textCtx.fillStyle = '#e57553';
              textCtx.fillText(badLabel, 180, 95);
              textCtx.font = '12px Fira San';
              textCtx.fillStyle = '#88b358';
              textCtx.fillText(goodLabel, 180, 105);
            }
          }
        }
      }
    };
  }

  private _getNaplesNumbers(): any {
    const obj = {};
    obj['goodNum'] = 803;
    obj['badNum'] = 431;
    obj['goodLabel'] = 'Good';
    obj['badLabel'] = 'Bad';
    obj['percent'] = '65%';
    return obj;
  }


  protected getDashboardData() {
    if (!this.widgets) {
      this.widgets = [];
    } else {
      this.widgets.length = 0;
    }
    this.widgets.push(this._build_SystemCapacity());
    this.widgets.push(this._build_SoftwareVersion());
    this.widgets.push(this._build_PoliciesHealth());
    this.widgets.push(this._build_Workloads());
    this.widgets.push(this._buildNaples());
  }


  removeLine(gridster) {
    gridster.setOption('lanes', --this.gridsterOptions.lanes)
      .reload();
  }
  addLine(gridster) {
    gridster.setOption('lanes', ++this.gridsterOptions.lanes)
      .reload();
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
