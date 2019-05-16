import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { WidgetsModule } from 'web-app-framework';

import { GuidesearchComponent } from '@app/components/search/guidedsearch/guidedsearch.component';

describe('GuidesearchComponent', () => {
  let component: GuidesearchComponent;
  let fixture: ComponentFixture<GuidesearchComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ GuidesearchComponent ],
      imports: [
        FormsModule,
        SharedModule,
        WidgetsModule,
        PrimengModule
      ]
    });
    });

  beforeEach(() => {
    fixture = TestBed.createComponent(GuidesearchComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
