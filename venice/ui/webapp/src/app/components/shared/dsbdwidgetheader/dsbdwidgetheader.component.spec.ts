import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import {WidgetsModule} from 'web-app-framework';

import { DsbdwidgetheaderComponent } from './dsbdwidgetheader.component';
import {MaterialdesignModule} from '@lib/materialdesign.module';

describe('DsbdwidgetheaderComponent', () => {
  let component: DsbdwidgetheaderComponent;
  let fixture: ComponentFixture<DsbdwidgetheaderComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DsbdwidgetheaderComponent ],
      imports: [
        WidgetsModule,
        MaterialdesignModule
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DsbdwidgetheaderComponent);
    component = fixture.componentInstance;
    component.icon = {
        margin: {
          top: '5px',
          left: '5px',
        },
        svgIcon: 'test.com'
    };
    component.id = 'test_id';
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
