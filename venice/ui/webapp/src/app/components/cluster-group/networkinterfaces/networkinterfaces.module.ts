import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {FormsModule, ReactiveFormsModule, } from '@angular/forms';
import {PrimengModule} from '@lib/primeng.module';
import {FlexLayoutModule} from '@angular/flex-layout';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {SharedModule} from '@components/shared/shared.module';
import {NetworkinterfacesComponent } from './networkinterfaces.component';
import {NetworkintersRoutingModule} from '@components/cluster-group/networkinterfaces/networkinterfaces.route';
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
      NetworkintersRoutingModule,
      SharedModule
    ],
    declarations: [NetworkinterfacesComponent]
  }
)
export class NetworkinterfacesModule { }
