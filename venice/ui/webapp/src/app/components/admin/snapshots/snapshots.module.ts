import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FlexLayoutModule } from '@angular/flex-layout';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';

/**-----
 Venice UI lib-  imports
 ------------------*/
import { SharedModule } from '@app/components/shared//shared.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';

/**-----
 Venice UI local module-  imports
 ------------------*/
import { SnapshotsRoutingModule } from './snapshots.route';
import { SnapshotsComponent } from './snapshots.component';

@NgModule({
  declarations: [SnapshotsComponent],
  imports: [
    CommonModule,

    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    FormsModule,
    ReactiveFormsModule,
    SharedModule,

    SnapshotsRoutingModule
  ]
})
export class SnapshotsModule { }
