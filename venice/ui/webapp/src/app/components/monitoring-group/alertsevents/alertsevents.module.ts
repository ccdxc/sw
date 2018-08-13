import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { AlertseventsComponent } from '@app/components/monitoring-group/alertsevents/alertsevents.component';
import { AlertsEventsRoutingModule } from '@app/components/monitoring-group/alertsevents/alertsevents.route';
/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared/shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';



@NgModule({
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,

    AlertsEventsRoutingModule,
    SharedModule
  ],
  declarations: [AlertseventsComponent]
})
export class AlertsEventsModule { }
