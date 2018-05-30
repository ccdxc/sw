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

@NgModule({
  imports: [
    CommonModule,
    FlexLayoutModule,
    MaterialdesignModule

  ],
  declarations: [PagebodyComponent,
    SpinnerComponent,
    DsbdwidgetheaderComponent,
    ModalheaderComponent,
    ModalbodyComponent,
    ModalitemComponent,
    ModalcontentComponent,
    ModalwidgetComponent
  ],
  exports: [
    PagebodyComponent,
    SpinnerComponent,
    DsbdwidgetheaderComponent,
    ModalheaderComponent,
    ModalbodyComponent,
    ModalitemComponent,
    ModalcontentComponent,
    ModalwidgetComponent
  ]
})
export class SharedModule { }
