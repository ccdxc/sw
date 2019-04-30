import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

import { RolloutsComponent } from './rollouts.component';
import { RolloutsRoutingModule } from './rollouts.route';

/**-----
 Venice UI lib-  imports
 ------------------*/
import { SharedModule } from '@app/components/shared//shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';

import { RolloutService } from '@app/services/generated/rollout.service';
import { ObjstoreService} from '@app/services/generated/objstore.service';
import { NewrolloutComponent } from './newrollout/newrollout.component';
import { RolloutstatusComponent } from './rolloutstatus/rolloutstatus.component';


@NgModule({
  declarations: [RolloutsComponent, NewrolloutComponent, RolloutstatusComponent],
  imports: [
    CommonModule,
    ReactiveFormsModule,
    FormsModule,
    RolloutsRoutingModule,

    PrimengModule,
    MaterialdesignModule,
    WidgetsModule,
    SharedModule,
    FlexLayoutModule
  ],
  providers: [
    RolloutService,
    ObjstoreService
  ]
})
export class RolloutsModule { }
