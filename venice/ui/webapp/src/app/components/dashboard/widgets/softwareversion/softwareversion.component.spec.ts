import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { WidgetsModule} from 'web-app-framework';
import { PrimengModule } from '@app/lib/primeng.module';
import {MaterialdesignModule} from '@lib/materialdesign.module';

import { SoftwareversionComponent } from './softwareversion.component';
import { DsbdwidgetheaderComponent } from '@app/components/shared/dsbdwidgetheader/dsbdwidgetheader.component';

describe('SoftwareversionComponent', () => {
  let component: SoftwareversionComponent;
  let fixture: ComponentFixture<SoftwareversionComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ SoftwareversionComponent, DsbdwidgetheaderComponent ],
      imports: [
        WidgetsModule,
        PrimengModule,
        MaterialdesignModule,
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(SoftwareversionComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
