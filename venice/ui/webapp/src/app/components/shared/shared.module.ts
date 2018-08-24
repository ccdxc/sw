import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PagebodyComponent } from './pagebody/pagebody.component';
import { FlexLayoutModule } from '@angular/flex-layout';
import { SpinnerComponent } from './spinner/spinner.component';
import { DsbdwidgetheaderComponent } from './dsbdwidgetheader/dsbdwidgetheader.component';
import { ModalheaderComponent } from './modal/modalheader/modalheader.component';
import { ModalbodyComponent } from './modal/modalbody/modalbody.component';
import { ModalitemComponent } from './modal/modalitem/modalitem.component';
import { ModalcontentComponent } from './modal/modalcontent/modalcontent.component';
import { ModalwidgetComponent } from './modal/modalwidget/modalwidget.component';

import { MaterialdesignModule } from '@lib/materialdesign.module';
import { TableheaderComponent } from '@app/components/shared/tableheader/tableheader.component';
import { LazyrenderComponent } from './lazyrender/lazyrender.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { AlertseventsComponent } from '@app/components/shared/alertsevents/alertsevents.component';
import { PrimengModule } from '@app/lib/primeng.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WhitespaceTrimDirective } from '@app/components/shared/directives/whitespacetrim.directive';

@NgModule({
  imports: [
    CommonModule,
    FlexLayoutModule,
    MaterialdesignModule,
    PrimengModule,
    ReactiveFormsModule,
    FormsModule,

  ],
  declarations: [PagebodyComponent,
    SpinnerComponent,
    DsbdwidgetheaderComponent,
    ModalheaderComponent,
    ModalbodyComponent,
    ModalitemComponent,
    ModalcontentComponent,
    ModalwidgetComponent,
    TableheaderComponent,
    LazyrenderComponent,
    PrettyDatePipe,
    AlertseventsComponent,
    WhitespaceTrimDirective
  ],
  exports: [
    PagebodyComponent,
    SpinnerComponent,
    DsbdwidgetheaderComponent,
    ModalheaderComponent,
    ModalbodyComponent,
    ModalitemComponent,
    ModalcontentComponent,
    ModalwidgetComponent,
    TableheaderComponent,
    LazyrenderComponent,
    PrettyDatePipe,
    AlertseventsComponent,
    WhitespaceTrimDirective
  ]
})
export class SharedModule { }
