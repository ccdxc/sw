import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {FormsModule, ReactiveFormsModule, } from '@angular/forms';
import {PrimengModule} from '@lib/primeng.module';
import {FlexLayoutModule} from '@angular/flex-layout';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {HostsRoutingModule} from '@components/cluster-group/hosts/hosts.route';
import {SharedModule} from '@components/shared/shared.module';
import {HostsComponent} from '@components/cluster-group/hosts//hosts.component';
import { NewhostComponent } from './newhost/newhost.component';
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
      HostsRoutingModule,
      SharedModule
    ],
    declarations: [HostsComponent, NewhostComponent]
  }
)

export class HostsModule {
}
