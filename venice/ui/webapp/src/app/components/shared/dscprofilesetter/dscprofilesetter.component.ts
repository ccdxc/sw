import {
  Component,
  OnInit,
  ViewEncapsulation,
  Input,
  Output,
  EventEmitter,
} from '@angular/core';
import {Utility} from '@common/Utility';
import {FormArray, FormControl, FormGroup} from '@angular/forms';
import { SelectItem } from 'primeng/api';

/**
 * Both DSC table page and DSC detail pages allow user to change DSC-profiles
 * This widget is common re-usable widget.
 * It is not restricted to use for DSC-Profile.  Basically, it is just dialog hosting a p-list.
 */

@Component({
  selector: 'app-dscprofilesetter',
  templateUrl: './dscprofilesetter.component.html',
  styleUrls: ['./dscprofilesetter.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class DscprofilesetterComponent implements OnInit {
  @Input() dialogMode = true;
  @Input() inEditMode: boolean;

  // save handler
  @Output() saveEmitter: EventEmitter<any> = new EventEmitter< any >();
  // cancel handler
  @Output() cancelEmitter: EventEmitter<object> = new EventEmitter<object>();

  @Input() selections: SelectItem[] = [];
  @Input() optionLabel: string = 'label';
  @Input() header: string = 'Set Profile To DSC';

  selectedOption: SelectItem;

  constructor() { }

  ngOnInit() {
  }

  onSave($event) {
    this.saveEmitter.emit(this.selectedOption);
  }

  onCancel($event) {
    this.cancelEmitter.emit();
  }

  cancel() {
    this.selectedOption = null;
  }

}
