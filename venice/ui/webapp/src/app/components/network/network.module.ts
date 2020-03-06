import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule} from '@angular/flex-layout';
import { SharedModule} from '@components/shared/shared.module';
import { NetworkRoutingModule } from './network.route';
import { WidgetsModule } from 'web-app-framework';
import { NetworkComponent } from './network.component';
import { NewnetworkComponent } from './newnetwork/newnetwork.component';

@NgModule({
  imports: [
    CommonModule,
      FormsModule,
      ReactiveFormsModule,
      PrimengModule,
      FlexLayoutModule,
      MaterialdesignModule,
      WidgetsModule,
      SharedModule,
      NetworkRoutingModule
  ],
  declarations: [NetworkComponent, NewnetworkComponent]
})


export class NetworkModule { }
