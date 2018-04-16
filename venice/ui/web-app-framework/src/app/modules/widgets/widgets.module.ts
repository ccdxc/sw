import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

import { MaterialModule } from '../../material.module';
import { PrimengModule } from '../../primeng.module';
import { ObjectUtils } from 'primeng/components/utils/objectutils';

import { BarchartComponent } from './d3charts/barchart/barchart.component';
import { PlotlyComponent } from './plotlychart/plotly/plotly.component';
import { ModuleUtility} from './ModuleUtility';
import { RecursivelistComponent } from './recursivelist/recursivelist/recursivelist.component';
import { PlotlyimageComponent } from './plotlychart/plotlyimage/plotlyimage.component';
import * as d3 from 'd3';
import { SearchboxComponent } from './searchbox/searchbox.component';

@NgModule({
  imports: [
    CommonModule,
    MaterialModule,
    PrimengModule,
    FormsModule
  ],
  declarations: [BarchartComponent, PlotlyComponent, RecursivelistComponent, PlotlyimageComponent, SearchboxComponent],
  exports: [
    BarchartComponent, // <-- add widgets to here
    PlotlyComponent,
    PlotlyimageComponent,
    RecursivelistComponent,
    SearchboxComponent
  ]
})
export class WidgetsModule { }
