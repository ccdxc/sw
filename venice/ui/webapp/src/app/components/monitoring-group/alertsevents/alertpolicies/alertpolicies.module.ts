import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

/**-----
 Venice UI -  imports
 ------------------*/
import { SharedModule } from '@app/components/shared//shared.module';
import { AlertpoliciesComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/alertpolicies.component';
import { AlertPoliciesRoutingModule } from '@app/components/monitoring-group/alertsevents/alertpolicies/alertpolicies.route';
import { ObjectpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/objectpolicy/objectpolicy.component';
import { MetricpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/metricpolicy/metricpolicy.component';
import { EventalertpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/eventalertpolicies/eventalertpolicies.component';
import { DestinationpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/destinations/destinations.component';
import { WidgetsModule } from 'web-app-framework';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { NeweventalertpolicyComponent } from '@app/components/monitoring-group/alertsevents/alertpolicies/eventalertpolicies/neweventalertpolicy/neweventalertpolicy.component';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,

    AlertPoliciesRoutingModule,
    SharedModule,
    WidgetsModule
  ],
  declarations: [AlertpoliciesComponent,
    ObjectpolicyComponent,
    MetricpolicyComponent,
    EventalertpolicyComponent,
    DestinationpolicyComponent,
    NeweventalertpolicyComponent
  ],
  providers: [
    MonitoringService
  ]
})
export class AlertPoliciesModule { }
