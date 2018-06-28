import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ObjectpolicyComponent } from './objectpolicy.component';

describe('ObjectpolicyComponent', () => {
  let component: ObjectpolicyComponent;
  let fixture: ComponentFixture<ObjectpolicyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ObjectpolicyComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ObjectpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
