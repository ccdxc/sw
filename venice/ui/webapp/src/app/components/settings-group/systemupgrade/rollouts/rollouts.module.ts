import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RolloutsComponent } from './rollouts.component';
import { RolloutsRoutingModule } from './rollouts.route';

/**-----
 Venice UI lib-  imports
 ------------------*/
 import { SharedModule } from '@app/components/shared//shared.module';
 import { MaterialdesignModule } from '@lib/materialdesign.module';
 import { PrimengModule } from '@lib/primeng.module';

 import { RolloutService } from '@app/services/generated/rollout.service';


@NgModule({
  declarations: [RolloutsComponent],
  imports: [
    CommonModule,
    RolloutsRoutingModule,

    PrimengModule,
    MaterialdesignModule,
    SharedModule
  ],
  providers: [
    RolloutService
  ]
})
export class RolloutsModule { }
