import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { AlertlistComponent } from './alertlist.component';
import { AlertlistitemComponent } from './alertlistitem.component';

@NgModule({
  imports: [
    CommonModule,
    FormsModule
    ],
  declarations: [AlertlistComponent, AlertlistitemComponent],
  exports: [AlertlistComponent, AlertlistitemComponent]
})
export class AlertlistModule { }

