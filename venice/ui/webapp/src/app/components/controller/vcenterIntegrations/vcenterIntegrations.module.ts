import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {FormsModule, ReactiveFormsModule, } from '@angular/forms';
import {PrimengModule} from '@lib/primeng.module';
import {FlexLayoutModule} from '@angular/flex-layout';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {VcenterIntegrationsRoutingModule} from '@components/controller/vcenterIntegrations/vcenterIntegrations.route';
import {SharedModule} from '@components/shared/shared.module';
import {NewVcenterIntegrationComponent} from './newVcenterIntegration/newVcenterIntegration.component';
import {VcenterIntegrationsComponent} from './vcenterIntegrations.component';
import { WidgetsModule } from 'web-app-framework';

@NgModule(
  {
    imports: [
      CommonModule,
      FormsModule,

      ReactiveFormsModule,
      PrimengModule,
      FlexLayoutModule,
      MaterialdesignModule,
      WidgetsModule,
      VcenterIntegrationsRoutingModule,
      SharedModule
    ],
    declarations: [VcenterIntegrationsComponent, NewVcenterIntegrationComponent]
  }
)

export class VcenterIntegrationsModule {
}
