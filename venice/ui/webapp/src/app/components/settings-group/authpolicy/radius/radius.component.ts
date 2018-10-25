import { Component, OnInit, ViewEncapsulation, Output, EventEmitter, Input, OnChanges, SimpleChanges } from '@angular/core';
import { AuthpolicybaseComponent } from '@app/components/settings-group/authpolicy/authpolicybase/authpolicybase.component';
import { Animations } from '@app/animations';
import { IAuthRadius, AuthRadius, AuthRadiusServer } from '@sdk/v1/models/generated/auth';
import { FormArray } from '@angular/forms';

@Component({
  selector: 'app-radius',
  templateUrl: './radius.component.html',
  styleUrls: ['./radius.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(mouseenter)': 'onMouseEnter()',
    '(mouseleave)': 'onMouseLeave()'
  },
  animations: [Animations]
})
export class RadiusComponent extends AuthpolicybaseComponent implements OnInit, OnChanges {
  isHover: boolean = false;
  radiusEditMode: boolean = false;
  inCreateMode: boolean = false;
  radiusObject: AuthRadius = new AuthRadius();

  @Input() radiusData: IAuthRadius;

  constructor() {
    super();
  }

  ngOnInit() {
    this.updateRadiusObject();
  }

  ngOnChanges(changes: SimpleChanges) {
    this.updateRadiusObject();
  }

  updateRadiusObject() {
    this.radiusObject.setValues(this.radiusData);
  }

  toggleEdit() {
    this.radiusEditMode = !this.radiusEditMode;
    if (this.radiusEditMode) {
      if (this.radiusObject.servers.length === 0) {
        const servers = this.radiusObject.$formGroup.get('servers') as FormArray;
        servers.insert(0, new AuthRadiusServer().$formGroup);
      }
    }
  }

  addServer() {
    const servers = this.radiusObject.$formGroup.get('servers') as FormArray;
    servers.insert(0, new AuthRadiusServer().$formGroup);
  }

  removeServer(index) {
    const servers = this.radiusObject.$formGroup.get('servers') as FormArray;
    if (servers.length > 1) {
      servers.removeAt(index);
    }
  }

  cancelEdit() {
    this.radiusEditMode = false;
    if (this.inCreateMode) {
      // create from is canceling,
      this.radiusData = null;
    }
    // Reset the radiusObject with the passed in data
    this.updateRadiusObject();
  }

  saveRadius() {
    // POST DATA
    this.radiusEditMode = false;
    // Reset the radiusObject with the passed in data
    this.updateRadiusObject();
  }

  createRadius() {
    this.radiusData = new AuthRadius();
    this.inCreateMode = true;
    this.radiusEditMode = true;
  }
}
