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
import { UniquerepeaterComponent } from './repeater/uniquerepeater/uniquerepeater.component';
import { PenuitabComponent } from './penuitabs/penuitab/penuitab.component';
import { PenuitabsComponent } from './penuitabs/penuitabs.component';
import { TabcontentComponent } from './penuitabs/tabcontent/tabcontent.component';
import { LabelDirective } from './directives/label.directive'

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
    RepeaterComponent,
    UniquerepeaterComponent,
    PenuitabComponent,
    PenuitabsComponent,
    TabcontentComponent,
    LabelDirective
  ],
  exports: [
    BarchartComponent, // <-- add widgets to here
    PlotlyComponent,
    PlotlyimageComponent,
    RecursivelistComponent,
    RepeaterComponent,
    UniquerepeaterComponent,
    PenuitabComponent,
    PenuitabsComponent,
    TabcontentComponent,
    LabelDirective
  ]
})
export class WidgetsModule { }
