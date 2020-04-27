import { Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation, OnChanges, SimpleChanges } from '@angular/core';
import { SelectItem } from 'primeng/api';

/**
 * Both DSC table page and DSC detail pages allow user to change DSC-profiles
 * This widget is common re-usable widget.
 * It is not restricted to use for DSC-Profile.  Basically, it is just dialog hosting a p-list.
 *
 * User clicks [save]  --> onSave() ( set disableDSCProfilerSetterSaveButton to true, diabled [save] button)
 *                      --> NaplesComponent.c.ts will call updateDSCProfilesWithBulkEdit() ( set saveDSCProfileOperationDone value to null )
 *                        --> TableViewEdit.bulkeditHelper()
 *                          --> NaplesComponent.onBulkEditSuccess()/onBulkEditFailure() (set saveDSCProfileOperationDone  value to true )
 *                            --> onNgChange() (reset disableDSCProfilerSetterSaveButton)
 *             [Cancel] --> onCancel() (just reset values)
 */

@Component({
  selector: 'app-dscprofilesetter',
  templateUrl: './dscprofilesetter.component.html',
  styleUrls: ['./dscprofilesetter.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class DscprofilesetterComponent implements OnInit, OnChanges {

  @Input() dialogMode = true;
  @Input() inEditMode: boolean;

  // save handler
  @Output() saveEmitter: EventEmitter<any> = new EventEmitter< any >();
  // cancel handler
  @Output() cancelEmitter: EventEmitter<object> = new EventEmitter<object>();

  @Input() selections: SelectItem[] = [];
  @Input() optionLabel: string = 'label';
  @Input() header: string = 'Set Profile To DSC';


  // This variable lets outter component (e.g Naples.component.ts) to inform DscprofilesetterComponent REST call is completed
  // For example, bulkedit on DSCs can be lengthy. We disable [save] button while runng REST call. Once server call is done, saveDSCProfileOperationDone value will be updated
  @Input() saveDSCProfileOperationDone: boolean ;

  //  VS-1553, we want to disable save button when UI emits a call to update records. It works along with this.saveDSCProfileOperationDone
  disableDSCProfilerSetterSaveButton: boolean = false;



  selectedOption: SelectItem;



  constructor() { }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {

  }

  ngOnChanges(changes: SimpleChanges): void {
    if (this.saveDSCProfileOperationDone != null && this.saveDSCProfileOperationDone !== undefined  ) {
      this.disableDSCProfilerSetterSaveButton = false;
    }
    if (changes.inEditMode) {
      this.disableDSCProfilerSetterSaveButton = ! this.inEditMode;
    }
  }

  onSave($event) {
    this.saveDSCProfileOperationDone = null;
    this.disableDSCProfilerSetterSaveButton = true; // disable [save] button to prevent user from clicking it multiple times
    this.saveEmitter.emit(this.selectedOption);
  }

  onCancel($event) {
    this.disableDSCProfilerSetterSaveButton = false;  // enable [save] button
    this.cancelEmitter.emit();
  }

  cancel() {
    this.selectedOption = null;
    this.onCancel(null);
  }

}
