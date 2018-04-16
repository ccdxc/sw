import { Component, OnInit, Input, Output, ViewEncapsulation, OnChanges } from '@angular/core';
import { ModuleUtility } from '../../ModuleUtility';
import { PlotlyComponent } from '../plotly/plotly.component';

@Component({
  selector: 'pw-plotlyimage',
  templateUrl: './plotlyimage.component.html',
  styleUrls: ['./plotlyimage.component.css'],
  encapsulation: ViewEncapsulation.None
})
export class PlotlyimageComponent implements OnInit, OnChanges {

  protected id_prefix = 'pw-PloyChartToImage_';

  @Input() imageWidth = 150;
  @Input() imageHeight = 60;
  @Input() fillcolor: '#ccabce';
  @Input() data: any;
  @Input() layout: any;
  @Input() options: any;
  @Input() styleClass: string = null;
  @Input() id: string = null;
  @Input() callback: any = null;

  dataset: any;
  options_default: any;
  layout_default: any;
  fillcolorOption: '#ccabce';

  callbackToImage: any;
  imagesource: string;
  isImageReady = false;
  setWidgetStyles: any;

  constructor() {
  }

  ngOnInit() {
    console.log('PlotlyimageComponent.ngInit()');
    if (!this.id) {
      this.id = this.id_prefix + ModuleUtility.s4() + ModuleUtility.s4();
    }
  }

  ngOnChanges() {
    this.setup();
  }

  genWidgetStyles() {
    const styles = {
      'width': this.imageWidth + 'px',
      'height': this.imageHeight + 'px'
    };
    // console.log(this.imageWidth, this.imageHeight);
    return styles;
  }

  protected setup() {
    this.setWidgetStyles = this.genWidgetStyles();
    this._setupDefaults();
    if (!this.options) {
      this.options = this.options_default;
    }
    if (!this.layout) {
      this.layout = this.layout_default;
    }
    if (!this.callback) {
      this.callback = this.callbackToImage;
    }
  }

  protected _setupDefaults() {
    this.isImageReady = false;
    if (this.fillcolor) {
      this.fillcolorOption = this.fillcolor;
    }
    // If passed in data is an array,
    // we assume it is in the data format for plotly
    if (!this.dataset) {
      this.dataset = [];
    }
    this.dataset.length = 0;
    if (Array.isArray(this.data)) {
      this.dataset = this.data;
    } else {
      this.dataset = [
        // passed in graph data
        {
          x: this.data.x,
          y: this.data.y,
          fill: 'tonexty',
          type: 'scatter',
          mode: 'none',
          fillcolor: this.fillcolorOption,
          line: {
            shape: 'spline',
          }
        }
      ];
    }

    this.layout_default = {
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

    this.options_default = { displayModeBar: false };
    const self = this;
    this.callbackToImage = function (Plotly, element) {
      const d3 = Plotly.d3;
      const img_png = d3.select('#' + this.id + '_pw-plotlychartimage-image-tag');
      Plotly.toImage(element, { height: self.imageHeight, width: self.imageWidth })
        .then(function (url) {
          self.imagesource = url;
          self.isImageReady = true;
        });
    };

  }


}
