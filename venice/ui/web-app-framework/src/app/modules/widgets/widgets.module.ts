import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MaterialModule } from '../../material.module';
import { PrimengModule } from '../../primeng.module';
import { BarchartComponent } from './d3charts/barchart/barchart.component';
import { PlotlyComponent } from './plotlychart/plotly/plotly.component';
import { PlotlyimageComponent } from './plotlychart/plotlyimage/plotlyimage.component';
import { RecursivelistComponent } from './recursivelist/recursivelist/recursivelist.component';
import { RepeaterComponent } from './repeater/repeater.component';
import { SearchboxComponent } from './searchbox/searchbox.component';
import { UniquerepeaterComponent } from './repeater/uniquerepeater/uniquerepeater.component';


@NgModule({
  imports: [
    CommonModule,
    MaterialModule,
    PrimengModule,
    FormsModule,
    ReactiveFormsModule,
    FlexLayoutModule
  ],
  declarations: [
    BarchartComponent,
    PlotlyComponent,
    RecursivelistComponent,
    PlotlyimageComponent,
    SearchboxComponent,
    RepeaterComponent,
    UniquerepeaterComponent
  ],
  exports: [
    BarchartComponent, // <-- add widgets to here
    PlotlyComponent,
    PlotlyimageComponent,
    RecursivelistComponent,
    SearchboxComponent,
    RepeaterComponent,
    UniquerepeaterComponent
  ]
})
export class WidgetsModule { }
