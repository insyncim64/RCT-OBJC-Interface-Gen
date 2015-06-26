@class Test;

@interface Test2 : NSObject

#pragma mark Public

- (instancetype)initWithInt:(jint)complexIndex
withTest:(Test *)complexInfo;

- (int)compareToWithId:(id)obj;

- (int)getComplexIndex;

- (Test *)getComplexInfo;

@end

#endif // _Test2_H_
