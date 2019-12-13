import { NgModule } from '@angular/core';
import {
  AccordionModule,
  AutoCompleteModule,
  BlockUIModule,
  BreadcrumbModule,
  SpinnerModule,
  CalendarModule,
  ChartModule,
  CheckboxModule,
  DataGridModule,
  DataTableModule,
  DropdownModule,
  FieldsetModule,
  GrowlModule,
  InputMaskModule,
  InputSwitchModule,
  InputTextareaModule,
  ListboxModule,
  MultiSelectModule,
  OverlayPanelModule,
  PanelModule,
  RadioButtonModule,
  SelectButtonModule,
  SharedModule,
  SliderModule,
  SplitButtonModule,
  StepsModule,
  ToolbarModule,
  TooltipModule,
  InputTextModule,
  ConfirmDialogModule,
  ChipsModule,
  DialogModule, ScrollPanelModule
} from 'primeng/primeng';
import { TableModule } from 'primeng/table';
import { ToastModule } from 'primeng/toast';
import { TreeModule } from 'primeng/tree';
import { TreeTableModule } from 'primeng/treetable';
import { PickListModule } from 'primeng/picklist';
import {FileUploadModule} from 'primeng/fileupload';


/**
 * This module control what primeNG widgets to use in our application.
 * We specify import/export here to avoid overloading app.module.ts.
 * In case, we don't use mprimeNG, we just simple take a small change in app.module.ts
 */
@NgModule({
  imports: [
    ChartModule, SplitButtonModule, StepsModule, AccordionModule, SpinnerModule, CalendarModule, PanelModule, DataTableModule,
    SharedModule, BlockUIModule, DropdownModule, SliderModule,
    ToolbarModule, FieldsetModule, InputTextModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
    MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, OverlayPanelModule,
    SelectButtonModule, TooltipModule, AutoCompleteModule, BreadcrumbModule, TableModule, InputTextModule, ToastModule, TreeModule, TreeTableModule, PickListModule, ConfirmDialogModule, FileUploadModule, ChipsModule, DialogModule, ScrollPanelModule
  ],
  exports: [
    ChartModule, SplitButtonModule, StepsModule, AccordionModule, SpinnerModule, CalendarModule, PanelModule, DataTableModule,
    SharedModule, BlockUIModule, DropdownModule, SliderModule,
    ToolbarModule, FieldsetModule, InputTextModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
    MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, OverlayPanelModule,
    SelectButtonModule, TooltipModule, AutoCompleteModule, BreadcrumbModule, TableModule, InputTextModule, ToastModule, TreeModule, TreeTableModule, PickListModule, ConfirmDialogModule, FileUploadModule, ChipsModule, DialogModule, ScrollPanelModule
  ],
  declarations: []
})
export class PrimengModule { }
