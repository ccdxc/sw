
import { Component, EventEmitter, Input, Output, OnInit, ViewEncapsulation, ViewChild, ElementRef, OnChanges } from '@angular/core';
import { ModuleUtility } from '../../ModuleUtility';
declare var Plotly: any;

/**
 * This is a plotly.js angular component.
 *
 *
 * How to use it:
 * <div class="test-plotly-chart-div">
 *   <pw-plotly-chart
 *       [data]="plotlyData"
 *       [layout]="plotlyLayout"
 *       [options]="plotlyOptions"
 *       [styleClass]="my_css_name_string"  //optional
 *       [id]="my_id_string" //optional
 *   ></pw-plotly-chart>
 *
 *  How it works:
 *  This widget depends on widget-invoker to provide data, layout, and options to render chart.
 *  Widget invoker can also provide "styleClass" and "id" to make chart HTML-DOM easier to access in application.
 *  This widget listens to change, it will then redraw the chart.
 *
 */
@Component({
  selector: 'pw-plotly-chart',
  templateUrl: './plotly.component.html',
  styleUrls: ['./plotly.component.css'],

  encapsulation: ViewEncapsulation.None
})
export class PlotlyComponent implements OnInit, OnChanges {

  @ViewChild('plotlychart') el: ElementRef;

  @Input() data: any;
  @Input() layout: any;
  @Input() options: any;
  @Input() styleClass: string = null;
  @Input() id: string = null;
  @Input() callback: any = null;

  protected id_prefix = 'pw-PloyChart_';


  constructor() { }

  ngOnInit() {
    if (!this.id) {
      this.id = this.id_prefix + this._s4() + this._s4();
    }
  }

  ngOnChanges() {
    if (this.data && this.layout && this.el) {
      const element = this.el.nativeElement;
      // NOTE: put try-catch here for UNIT test, as ChromeHeadless can not have canvas. UT with Chrome will work
      try {

        const self = this;
        Plotly.newPlot(element, this.data, this.layout, this.options).then(function() {
          if (self.callback != null) {
            self.callback(Plotly, element);
          }
        });
      } catch (error) {
        if (!ModuleUtility.isChromeHeadless()) {
          console.log('PlotlyComponent.ngOnChanges()\n' + error);
          console.log(error.stack);
        }
      }
    }
  }

  /**
   * Utility function
   */
  protected _s4(): string {
    return ModuleUtility.s4();
  }



  /**
   * This API serves HTML template.
   * It adds class to chart HTML-DOM element
   */
  getStyleClass(): string {
    return (this.styleClass) ? this.styleClass : 'pw-plotly-chart';
  }

}
