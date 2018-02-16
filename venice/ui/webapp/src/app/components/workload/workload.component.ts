import { Component, OnInit, OnDestroy, ViewEncapsulation, ViewChild, ElementRef } from '@angular/core';

import { ControllerService } from '../../services/controller.service';
import { Eventtypes } from '../../enum/eventtypes.enum';
import { Logintypes } from '../../enum/logintypes.enum';
import { Utility } from '../../common/Utility';

import { WorkloadService } from '../../services/workload.service';
import { BaseComponent } from '../base/base.component';
import { MockDataUtil } from '@common/MockDataUtil';


@Component({
  selector: 'app-workload',
  templateUrl: './workload.component.html',
  styleUrls: ['./workload.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadComponent extends BaseComponent implements OnInit, OnDestroy {

  items: any;
  protected content: any = {
    'totalworkloads': {
      data: [],
      layout: {},
      option: {},
      text: '',
      id: 'totalworkloads'
    },
    'newworkloads': {
      data: [],
      option: {},
      text: '',
      id: 'newworkloads',
      layout: {
        title: 'Workload Alerts Trend',
        margin: { t: 0 }
      }
    },
    'plotlychart': {
      'plotlyData': [],
      'plotlyLayout': {},
      'plotlyOptions': {}
    },
    'plotly3Dchart': {
      'plotlyData':  [{
        z: MockDataUtil.getPlotly3DChartData(),
        type: 'surface'
      }],
      'plotlyLayout': {
        title: 'Workloads Resource consumption' ,
        scene: {
          xaxis: {
            title: 'X Axis',
            titlefont: {
               color: 'red',
               family: 'Arial, Open Sans',
               size: 12
            }
          },
          yaxis: {
            title: 'Y Axis',
            titlefont: {
               color: 'red',
               family: 'Arial, Open Sans',
               size: 12
            }
          },
          zaxis: {
            title: 'Z Axis',
            titlefont: {
               color: 'red',
               family: 'Arial, Open Sans',
               size: 12
            }
          }
        },
        autosize: false,
        width: 500,
        height: 500,
        margin: {
          l: 65,
          r: 50,
          b: 65,
          t: 90
        }
      },

      'plotlyOptions': this.getPlotlyChartConfiguration()
    }
  };

  constructor(
    private _workloadService: WorkloadService,
    protected _controllerService: ControllerService) {
    super(_controllerService);
    this._testPlotlyChart();
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      const self = this;
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
        'component': 'WorkloadComponent', 'state':
          Eventtypes.COMPONENT_INIT
      });
      this.getItems();
      // set a timeout in 1 second.  The timeout function has a setInterval to generate mock-data in every 3 seconds
      setTimeout(() => {
        this.generateData();
        // change the data periodically
        setInterval(() => this.generateData(), 3000);
      }, 10);
    }
  }
  ngOnDestroy() {
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'WorkloadComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getLineSplineData(lenPlotly: number): any {
    const mockPlotlyX = this.generatePlotlyMockData(lenPlotly);
    const mockPlotlyY = this.generatePlotlyMockData(lenPlotly);

    mockPlotlyX.sort();
    const xyObj = {
      x: mockPlotlyX,
      y: mockPlotlyY
    };
    return xyObj;
  }
  generateData() {
    const len = (8 + Math.floor(Math.random() * 10));
    const totalXYObj = this.getLineSplineData(len);
    const totalXYData =  {
      x: totalXYObj.x,
      y: totalXYObj.y,
      fill: 'tonexty',
      type: 'scatter',
      mode: 'none',
      line: {
        shape: 'spline',
        color: 'rgb(255, 157, 98)'
      }
    };
    this.content.totalworkloads.data = [totalXYData];
    const newXYObj = this.getLineSplineData(len);
    const newXYData =  {
      x: newXYObj.x,
      y: newXYObj.y,
      fill: 'tozeroy',
      type: 'scatter',
      mode: 'none',
      line: {
        shape: 'spline',
        color: 'rgb(107, 295, 67)'
      }
    };
    this.content.newworkloads.data = [this.getLineSplineData(len)];

    this.content.totalworkloads.text = Utility.getRandomInt(1000, 10000) + ' Workloads';
    this.content.newworkloads.text = Utility.getRandomInt(10, 100) + ' New Workloads';

    const lenPlotly = (8 + Math.floor(Math.random() * 10));


    const myXYObj = this.getLineSplineData(lenPlotly);
    const tempTrace = {
      x: myXYObj.x,
      y: myXYObj.y,
      fill: 'tonexty',
      type: 'scatter',
      mode: 'none',
      line: {
        shape: 'spline',
        color: 'rgb(157, 255, 98)'
      }
    };
    this.content.plotlychart.plotlyData = [tempTrace];
  }

  generateBarchartMockData(len: number) {
    const chartData = [];
    for (let i = 0; i < len; i++) {
      chartData.push([
        `Index ${i}`,
        Math.floor(Math.random() * 100)
      ]);
    }
    return chartData;
  }

  generatePlotlyMockData(len: number) {
    const chartData = [];
    for (let i = 0; i < len; i++) {
      chartData.push( Math.floor(Math.random() * 100));
    }
    return chartData;
  }

  getItems() {

    this._workloadService.getItems().subscribe(
      data => {
        // Publish AJAX-END Event

        const isRESTPassed = Utility.isRESTFailed(data);
        if (isRESTPassed) {
          // process server response
          const isLoginPassed = Utility.isRESTFailed(data);
          if (isLoginPassed) {
            this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_ITEMS' });
            this.items = data.Items;
          } else {
            this.errorMessage = 'Failed to get items! ' + Utility.getRESTMessage(data);
          }
        } else {
          this.errorMessage = 'Failed to get items!  Please try again later ' + Utility.getRESTMessage(data);
          this._controllerService.publish(Eventtypes.AJAX_END, { 'ajax': 'end', 'name': 'WORKLOAD_GET_ITEMS' });
        }
        this.successMessage = '';
      },
      err => {
        this.successMessage = '';
        this.errorMessage = 'Failed to get items! ' + err;
        this.error(err);
      }
    );
  }



  private _testPlotlyChart() {
    const trace2 = {
      x: [1, 2, 3, 4],
      y: [3, 5, 1, 7],
      fill: 'tonexty',
      type: 'scatter',
      mode: 'none',
      line: {
        shape: 'spline',
        color: 'rgb(157, 255, 98)'
      }
    };

      /* let layout = {
        title: 'Workload Alerts Trend',
        margin: { t: 0 }
      }; */

      this.content.plotlychart.plotlyData = [trace2];
      this.content.plotlychart.plotlyLayout = this.getSimpleChartLayout('Workload Alerts Trend');
      this.content.plotlychart.plotlyOptions = { displayModeBar: false };


  }

  getPlotlyChartConfiguration(): any {
    const defaultPlotlyConfiguration = { modeBarButtonsToRemove: ['sendDataToCloud', 'autoScale2d', 'hoverClosestCartesian', 'hoverCompareCartesian', 'lasso2d', 'select2d'], displaylogo: false, showTips: true };
    return defaultPlotlyConfiguration;
  }

  getSimpleChartLayout(titleString: string): any {
    return {
      title: titleString,
      margin: {
        t: 50,

      }
    };
  }

  workloadClickHandler(id) {
    alert(id);
  }
}
