import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { MaterialModule } from '../../material.module';

import { BarchartComponent } from './d3charts/barchart/barchart.component';
import { PlotlyComponent } from './plotlychart/plotly/plotly.component';
import { ModuleUtility} from './ModuleUtility';
import { RecursivelistComponent } from './recursivelist/recursivelist/recursivelist.component';
import { PlotlyimageComponent } from './plotlychart/plotlyimage/plotlyimage.component';

@NgModule({
  imports: [
    CommonModule,
    MaterialModule
  ],
  declarations: [BarchartComponent, PlotlyComponent, RecursivelistComponent, PlotlyimageComponent],
  exports: [
    BarchartComponent, // <-- add widgets to here
    PlotlyComponent,
    PlotlyimageComponent,
    RecursivelistComponent
  ]
})
export class WidgetsModule { }
