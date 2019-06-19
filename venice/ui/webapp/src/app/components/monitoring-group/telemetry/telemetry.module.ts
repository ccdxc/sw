import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

/**-----
 Venice UI -  imports
 ------------------*/
 import { SharedModule } from '@app/components/shared/shared.module';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';
 import { WidgetsModule } from 'web-app-framework';
import { TelemetryComponent } from './telemetry.component';
import { TelemetryRoutingModule } from './telemetry.route';
import { TelemetrychartComponent } from './telemetrychart/telemetrychart.component';
import { TelemetrycharteditComponent } from './telemetrychart-edit/telemetrychartedit.component';
import { TelemetrychartviewComponent } from './telemetrychart-view/telemetrychartview.component';

@NgModule({
  declarations: [
    TelemetryComponent,
    TelemetrychartComponent,
    TelemetrycharteditComponent,
    TelemetrychartviewComponent
  ],
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,
    SharedModule,
    TelemetryRoutingModule
  ]
})
export class TelemetryModule { }
