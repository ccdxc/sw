import { Component, OnInit, ViewEncapsulation, Output, EventEmitter, Input } from '@angular/core';
import { AuthpolicybaseComponent } from '@app/components/settings-group/authpolicy/authpolicybase/authpolicybase.component';
import { IAuthLocal } from '@sdk/v1/models/generated/auth';

@Component({
  selector: 'app-local',
  templateUrl: './local.component.html',
  styleUrls: ['./local.component.scss'],
  encapsulation: ViewEncapsulation.None,
  host: {
    '(mouseenter)': 'onMouseEnter()',
    '(mouseleave)': 'onMouseLeave()'
  }
})
export class LocalComponent extends AuthpolicybaseComponent implements OnInit {
  @Input() localData: IAuthLocal;

  constructor() {
    super();
  }

  ngOnInit() {
  }

}
