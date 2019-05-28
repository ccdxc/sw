import {
  Component,
  OnInit,
  ViewEncapsulation,
  Input,
  OnChanges,
  SimpleChanges,
  Output,
  EventEmitter,
} from '@angular/core';
import {LabelEditorMetadataModel, LabelEditorModel, VeniceObject} from './index';
import {Utility} from '@common/Utility';

// Label Editor Widget
// This widget will allow user to edit the key value pair in meta.labels
// Example usage: webapp/src/app/components/cluster-group/naples/naplesdetail/naplesdetail.component.html
@Component({
  selector: 'app-labeleditor',
  templateUrl: './labeleditor.component.html',
  styleUrls: ['./labeleditor.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class LabeleditorComponent implements OnInit, OnChanges {
  models: LabelEditorModel[];
  usableArray = [];
  usableJson = {};
  // the object user wants to edit. Must contains meta.labels
  @Input() data: VeniceObject;
  // meta data that controls the behavior of this widget. E.g. enable save, enable delete, enable edit, enable cancel
  @Input() metadata: LabelEditorMetadataModel;
  // In the future, some labels may be not editable and not deletable. Widget user can override this buildModel() API to customize it.
  @Input() myBuildModel: Function;
  // inLabelEditMode is contolling the dialog popup
  @Input() inLabelEditMode: boolean;

  // get edited meta.labels as an array format
  @Output() arrayValue = new EventEmitter();
  // get edited meta.labels as an obj format
  @Output() jsonValue = new EventEmitter();
  // save handler
  @Output() saveEmitter: EventEmitter<object> = new EventEmitter<object>();
  // cancel handler
  @Output() cancelEmitter: EventEmitter<object> = new EventEmitter<object>();

  formValidated: boolean = true;

  constructor() {
  }

  /**
   * Helper function to determine if obj exists and if the key is in the obj
   * @param obj
   * @param key
   */
  hasProp(obj, key): boolean {
    if (!obj) {
      return false;
    }
    return obj.hasOwnProperty(key);
  }

  /**
   * Called everytime when the row prop changed
   */
  onObjectEdit() {
    const array = [];
    const json = {};
    for (const prop of this.models) {
      prop.key = prop.key.trim();
      prop.value = prop.value.trim();
      const newProp = {key: prop.key, value: prop.value};
      if (prop.key === '' || prop.value === '') {
        continue;
      }
      array.push(newProp);
      json[prop.key] = prop.value;
    }

    this.arrayValue.emit(array);
    this.usableArray = array;
    this.jsonValue.emit(json);
    this.usableJson = json;
  }

  /**
   * Handle add row pop logic
   */
  addProperty() {
    this.models.push(this.buildModel('', ''));
  }

  /**
   * Handle delete row prop logic
   * @param prop
   */
  delete(prop: LabelEditorModel) {
    const index: number = this.models.indexOf(prop);
    if (index === 0 && this.models.length === 1) {
      // Don't allow deletion of the last model, just clear its values out.
      this.models[0].key = '';
      this.models[0].value = '';
    } else if (index !== -1) {
      this.models.splice(index, 1);
    }
    this.onObjectEdit();
  }

  /**
   * Helper function to get class for key input
   * @param prop
   */
  getKeyClass(prop: LabelEditorModel): string {
    if (!prop) {
      return '';
    }
    if (prop.keyClass) {
      return prop.keyClass;
    } else if (this.metadata.keysClasses) {
      return this.metadata.keysClasses;
    } else {
      return 'label-editor-input-key';
    }
  }

  /**
   * Helper function to get class for key input with error if the input is not valid
   * @param prop
   */
  getKeyClassWithValidator(prop: LabelEditorModel): string {
    return this.validate(prop.key) ? this.getKeyClass(prop) : `${this.getKeyClass(prop)} error`;
  }


  ngOnInit() {
    if (!this.models) {
      this.models = [];
    }
  }

  /**
   * Build LabelEditorModel models from meta.labels
   */
  buildModels(): LabelEditorModel[] {
    const res = [];
    if (this.data.meta.labels && !Utility.getLodash().isEmpty(this.data.meta.labels)) {
      Object.entries(this.data.meta.labels).forEach(([key, value]) => {
        res.push(this.buildModel(key, value));
      });
    } else {
      res.push(this.buildModel('', ''));
    }
    return res;
  }

  /**
   * Build LabelEditorModel from key and value
   * @param key
   * @param value
   */
  buildModel(key: string, value: string): LabelEditorModel {
    if (this.myBuildModel) {
      return this.myBuildModel(key, value);
    } else {
      return {
        key: key,
        keyEditable: true,
        value: value,
        valueEditable: true,
        element: 'input',
        inputType: 'text',
        deletable: true,
        keyPlaceholder: 'key',
        valuePlaceholder: 'value'
      } as LabelEditorModel;
    }
  }

  /**
   * Build default LabelEditorMetadataModel
   */
  buildDefaultMetadataModel(): LabelEditorMetadataModel {
    const myTitle = (this.data) ? this.data.meta.name + ' Labels' : 'Label Editor';
    const labelEditorMetadataModel = {
      title: myTitle,
      keysEditable: true,
      valuesEditable: true,
      propsDeletable: true,
      extendable: true,
      save: true,
      cancel: true,
    };
    return labelEditorMetadataModel;
  }

  ngOnChanges(changes: SimpleChanges) {
    if (!this.data) {
      // data not loaded
      if (!this.models) {
        this.models = [
          this.buildModel('', '')
        ];
      }
      if (!this.metadata) {
        this.metadata = this.buildDefaultMetadataModel();
      }
    } else {
      // data loaded, use real data
      this.init();
      if (!this.metadata) {
        this.metadata = this.buildDefaultMetadataModel();
      }
    }
  }

  /**
   * Data init
   */
  init() {
    this.usableJson = {};
    this.usableArray = [];
    this.models = this.buildModels();
    this.onObjectEdit();
  }

  /**
   * Save button logic
   */
  save() {
    // save fields, the saveEmitter will handle the http call
    this.saveEmitter.emit(this.usableJson);
  }

  /**
   * Cancel button logic
   */
  cancel() {
    // reset fields
    this.init();
    this.cancelEmitter.emit();
  }

  /**
   * Validate the input.
   * Currently support duplicate key detection.
   * @param key
   */
  validate(key: string): boolean {
    if (!key || key === '') {
      return true;
    }
    let counter = 0;
    this.models.map(prop => {
      if (prop.key === key) { counter++; }
    });
    if (counter >= 2) {
      this.formValidated = false;
      return false;
    }
    this.formValidated = true;
    return true;
  }

  /**
   * On add button keydown
   * @param $event
   */
  onAddKeydown($event) {
    if ($event && $event.which === 13) {
      this.addProperty();
    }
  }

  /**
   * On delete button keydown
   * @param $event
   * @param prop
   */
  onDeleteKeydown($event, prop: LabelEditorModel) {
    if ($event && $event.which === 13) {
      this.delete(prop);
    }
  }
}
