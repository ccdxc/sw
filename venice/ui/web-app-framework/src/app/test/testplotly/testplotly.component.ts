import { Component, OnInit, ViewChild, ElementRef } from '@angular/core';
import { ModuleUtility } from '@modules/widgets/ModuleUtility';

@Component({
  selector: 'app-testplotly',
  templateUrl: './testplotly.component.html',
  styleUrls: ['./testplotly.component.css']
})
export class TestplotlyComponent implements OnInit {

  @ViewChild('chart') el: ElementRef;

  public plotlyLayout: any;
  public plotlyData: any;
  public plotlyOptions: any;

  public plotly3DLayout: any;
  public plotly3DData: any;
  public plotly3DOptions: any;

  public plotlyLayoutAnimation: any;
  public plotlyDataAnimation: any;
  public plotlyOptionsAnimation: any;
  public plotlyCallbackAnimation: any;

  public plotlyDataFrame: any;
  public plotlyLayoutFrame: any;
  public plotlyOptionsFrame: any;
  public plotlyCallbackFrame: any;

  public plotlyDataToImage: any;
  public plotlyLayoutToImage: any;
  public plotlyOptionsToImage: any;
  public plotlyCallbackToImage: any;
  public plotlyChartToImageShow = true;
  public plotlyToImageSrc: any;

  public plotlyDataToImageWidget: any;
  public plotlyDataToImageWidget_width = 200;
  public plotlyDataToImageWidget_height = 100;
  public plotlyDataToImageWidget_fillcolor = '#abc000';
  public plotlyFlex_fillcolor1 = '#ff0000';
  public plotlyFlex_fillcolor2 = '#00ff00';
  public plotlyFlex_fillcolor3 = '#0000ff';
  public plotlyFlex_fillcolor4 = '#abc000';
  public plotlyFlex_fillcolor5 = '#000abc';

  public plotlyDataToImageWidgetOption: any;
  public plotlyLayoutToImageWidgetOptions: any;
  public plotlyOptionsToImageWidgetOptions: any;

  public layout_setup: any;

  constructor() { }

  ngOnInit() {

    setTimeout(() => {
      this.generateData();

      // change the data periodically
      setInterval(() => this.generateData(), 3000);
    }, 1000);
    this._testPlotlyChart();
    this._testPlotly3DChart();
    this._testPlotlyAnimation();
    this._testPlotlyFrame();
    this._testPlotlyToImage();
    this._testPlotlyToImageWidget();
    this._testPlotlyToImageWidgetOptions();
  }

  private _testPlotlyChart() {
    this.plotlyData = [
      {
        x: [1, 2, 3, 4, 5],
        y: [1, 2, 4, 8, 16]
      }
    ];
    this.plotlyLayout = {
      margin: { t: 0 }
    };

    // var defaultPlotlyConfiguration = { modeBarButtonsToRemove: ['sendDataToCloud', 'autoScale2d', 'hoverClosestCartesian', 'hoverCompareCartesian', 'lasso2d', 'select2d'], displaylogo: false, showTips: true };
    const defaultPlotlyConfiguration = { modeBarButtonsToRemove: ['sendDataToCloud'], displaylogo: false, showTips: true, displayModeBar: true };

    this.plotlyOptions = defaultPlotlyConfiguration;
  }
  _testPlotlyToImageWidgetOptions() {
    this.plotlyDataToImageWidgetOption = [
      // passed in graph data
      {
        x: [1, 2, 5, 6],
        y: [2, 5, 3, 7],
        type: 'scatter',
        mode: 'lines',
        line: {
          shape: 'spline',
          // shape: 'linear'
          color: '#7db1ea'
        }
      },
      // second trace to add marker on last point
      {
        x: [6],
        y: [7],
        type: 'scatter',
        mode: 'marker',
        line: {
          color: '#7db1ea'
        }
      }
    ];
    this.plotlyLayoutToImageWidgetOptions = {
      showlegend: false,
      paper_bgcolor: 'rgba(0,0,0,0)',
      plot_bgcolor: 'rgba(0,0,0,0)',
      autosize: true,
      margin: {
        t: 7,
        l: 4.5,
        r: 13.5,
        b: 9.5
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
    this.plotlyOptionsToImageWidgetOptions = {
      displayModeBar: false
    };
  }
  _testPlotlyToImageWidget() {
    this.plotlyDataToImageWidget = {
      x: [1, 2, 5, 6],
      y: [2, 5, 3, 7],
    };
  }

  _testPlotlyToImage() {
    this.plotlyDataToImage = [
      {
        x: [1, 2, 3, 4],
        y: [3, 5, 1, 7],
        fill: 'tonexty',
        type: 'scatter',
        mode: 'none',
        fillcolor: '#ab63fa',
        line: {
          shape: 'spline',
          color: '#ccabce'
        }
      }
    ];
    this.plotlyLayoutToImage = {
      autosize: true,
      margin: {
        t: 2,
        l: 2,
        r: 2,
        b: 2
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

    this.plotlyOptionsToImage = { displayModeBar: false };
    const self = this;
    this.plotlyCallbackToImage = function(Plotly, element) {
      const d3 = Plotly.d3;

      const img_png = d3.select('#png-export');
      Plotly.toImage(element, { height: 60, width: 150 })
        .then(function(url) {
          self.plotlyToImageSrc = url;
          self.plotlyChartToImageShow = false;
        });
    };

  }

  _testPlotlyAnimation() {
    this.plotlyDataAnimation = [{
      y: new Array(100).fill(0).map((d, i) =>
        Math.pow(i / 50, 2) + Math.sin(i * 4)
      ),
      line: { dash: '0px 7200px' }
    }];

    this.plotlyLayoutAnimation = {
      margin: { t: 0 }
    };

    const defaultPlotlyConfiguration = { modeBarButtonsToRemove: ['sendDataToCloud'], displaylogo: false, showTips: true, displayModeBar: true };

    this.plotlyOptionsAnimation = defaultPlotlyConfiguration;

    this.plotlyCallbackAnimation = function(Plotly, element) {
      console.log('plotlyCallbackAnimation called');
      Plotly.animate(element,
        [{ data: [{ 'line.dash': '7200px 0px' }] }],
        {
          frame: { duration: 5000, redraw: false },
          transition: { duration: 5000 }
        }
      );
    };
  }

  _testPlotlyFrame() {
    const data = {
      x: [1, 3, 5, 7],
      y: [20, 14, 23, 50],
      type: 'line'
    };
    const data1 = {
      x: ['Storage', 'CPU', 'Memory', 'Networks'],
      y: [20, 14, 23, 50],
      type: 'bar'
    };
    this.plotlyDataFrame = [
      data1
    ];
    this.plotlyLayoutFrame = {
      width: 450,
      height: 300,
      margin: {
        t: 10,
        l: 20,
        r: 10,
        b: 20
      },
      yaxis: {
        autorange: true,
        showgrid: false,
        zeroline: false,
        showline: false,
        autotick: true,
        ticks: '',
        showticklabels: false
      }
    };
    const defaultPlotlyConfiguration = { modeBarButtonsToRemove: ['sendDataToCloud'], displaylogo: false, showTips: true, displayModeBar: true };

    this.plotlyOptionsFrame = defaultPlotlyConfiguration;

    const myFrames = [];
    const frameName = 'myFrameName';
    const groupName = 'myGroup';
    const mode = 'afterall';
    myFrames.push({
      name: frameName,
      data: [{ x: data.x, y: data.y }],
      group: 'mygroup'
    });

    // TODO: animation fails (bar/pie) can not have animation yet.
    this.plotlyCallbackFrame = function(Plotly, element) {
      console.log('plotlyCallbackFrame called');
      Plotly.addFrames(element, myFrames).then(function() {
        Plotly.animate(element, null, { mode: 'next' });
        Plotly.animate(element, groupName, {
          transition: {
            duration: 5000,
            easing: 'linear'
          },
          frame: {
            duration: 500,
            redraw: true,
          },
          mode: mode
        });
      });
      /* Plotly.animate(element, null, {mode: 'next'});
      Plotly.animate(element, groupName, {
        transition: {
          duration: 500,
          easing: 'linear'
        },
        frame: {
          duration: 500,
          redraw: false,
        },
        mode: mode
      }); */
    };
  }

  /**
   * Learn from: https://jsfiddle.net/27cfgLet/3/
   */
  private _testPlotly3DChart() {
    this.plotly3DData = [{
      z: ModuleUtility.getPlotly3DChartData(),
      type: 'surface'
    }];
    this.plotly3DLayout = {
      title: 'Mt Bruno Elevation',
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
    };
    this.plotly3DOptions = this.getPlotlyChartConfiguration();
  }

  getPlotlyChartConfiguration(): any {
    const defaultPlotlyConfiguration = { modeBarButtonsToRemove: ['sendDataToCloud'], displaylogo: false, showTips: true, displayModeBar: true };
    return defaultPlotlyConfiguration;

  }



  generateData() {
    /* this.plotlyData = [{
      x: [1, 2, 3, 4, 5],
      y: [1, 2, 4, 8, 16]
    }]; */
    const lenPlotly = (8 + Math.floor(Math.random() * 10));
    const mockPlotlyX = this.generatePlotlyMockData(lenPlotly);
    const mockPlotlyY = this.generatePlotlyMockData(lenPlotly);
    this.plotlyData = [{
      x: mockPlotlyX,
      y: mockPlotlyY
    }];
    // test plotlyChartToImage with timer
    /*
    this.plotlyDataToImageWidget = {
      x: mockPlotlyX,
      y: mockPlotlyY
     };
     //console.log ("x", mockPlotlyX);
     //console.log ("y", mockPlotlyY);
     //*/
  }

  generatePlotlyMockData(len: number) {
    const chartData = [];
    for (let i = 0; i < len; i++) {
      chartData.push(Math.floor(Math.random() * 100));
    }
    return chartData;
  }


  buildLocalChart() {
    const element = this.el.nativeElement;
    const data = [{
      x: [1, 2, 3, 4, 5],
      y: [1, 2, 4, 8, 16]
    }];
    const style = {
      margin: { t: 0 }
    };
    Plotly.plot(element, data, style);
  }


}
