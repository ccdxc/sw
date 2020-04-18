import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

import { PrimengModule } from '@lib/primeng.module';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { FlexLayoutModule } from '@angular/flex-layout';

/**-----
 Venice UI -  imports
 ------------------*/
import { NaplesComponent } from './naples.component';
import { NaplesRoutingModule } from './naples.route';
import { SharedModule } from '@app/components/shared//shared.module';
import { NaplesdetailComponent } from './naplesdetail/naplesdetail.component';
import { NaplesdetailchartsComponent } from './naplesdetail/widgets/naplesdetailcharts.component';
import { NaplesdetailstatsComponent } from './naplesdetail/widgets/naplesdetailstats.component';
import { NaplesdetailIftopologyComponent } from './naplesdetail/widgets/naplesdetailiftopology.component';
import { NaplesdetailIftableComponent } from './naplesdetail/widgets/naplesdetailiftable.component';
import { NaplesdetailIfstatsComponent } from './naplesdetail/widgets/naplesdetailifstats.component';

// import { NodedetailComponent } from './nodedetail/nodedetail.component';

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    PrimengModule,
    FlexLayoutModule,
    MaterialdesignModule,
    NaplesRoutingModule,
    SharedModule
  ],
  declarations: [
    NaplesComponent,
    NaplesdetailComponent,
    NaplesdetailchartsComponent,
    NaplesdetailstatsComponent,
    NaplesdetailIftopologyComponent,
    NaplesdetailIftableComponent,
    NaplesdetailIfstatsComponent
  ]
})
export class NaplesModule { }
