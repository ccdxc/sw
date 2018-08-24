import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { AlertseventspageComponent } from '@app/components/monitoring-group/alertsevents/alertseventspage.component';
import { AlertsEventsPageRoutingModule } from '@app/components/monitoring-group/alertsevents/alertseventspage.route';
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

    AlertsEventsPageRoutingModule,
    SharedModule
  ],
  declarations: [AlertseventspageComponent]
})
export class AlertsEventsPageModule { }
