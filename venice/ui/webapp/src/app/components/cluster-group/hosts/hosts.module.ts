import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {FormsModule} from '@angular/forms';
import {PrimengModule} from '@lib/primeng.module';
import {FlexLayoutModule} from '@angular/flex-layout';
import {MaterialdesignModule} from '@lib/materialdesign.module';
import {HostsRoutingModule} from '@components/cluster-group/hosts/hosts.route';
import {SharedModule} from '@components/shared/shared.module';
import {HostsComponent} from '@components/cluster-group/hosts//hosts.component';

@NgModule(
  {
    imports: [
      CommonModule,
      FormsModule,

      PrimengModule,
      FlexLayoutModule,
      MaterialdesignModule,

      HostsRoutingModule,
      SharedModule
    ],
    declarations: [HostsComponent]
  }
)

export class HostsModule {
}
