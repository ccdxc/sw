import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PagebodyComponent} from './pagebody/pagebody.component';
import { FlexLayoutModule } from '@angular/flex-layout';
import { SpinnerComponent } from './spinner/spinner.component';
import { DsbdwidgetheaderComponent } from './dsbdwidgetheader/dsbdwidgetheader.component';
import { MaterialdesignModule } from '@lib/materialdesign.module';

@NgModule({
  imports: [
    CommonModule,
    FlexLayoutModule,
    MaterialdesignModule

  ],
  declarations: [PagebodyComponent, SpinnerComponent, DsbdwidgetheaderComponent],
  exports: [
      PagebodyComponent,
      SpinnerComponent,
      DsbdwidgetheaderComponent
  ]
})
export class SharedModule { }
